/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <cutils/sockets.h>
#include <cutils/android_reboot.h>
#include <cutils/list.h>
#include <sys/reboot.h>
#include <unistd.h>

#if HAVE_GLIBC
#include <linux/reboot.h>
#include <sys/syscall.h>
#define __reboot(...) syscall(__NR_reboot,__VA_ARGS__)
#endif

#ifdef BUILD_EUREKA
#include <string.h>
#endif

#include "init.h"
#include "util.h"
#include "log.h"

static int signal_fd = -1;
static int signal_recv_fd = -1;

static void sigchld_handler(int s)
{
    write(signal_fd, &s, 1);
}

#define CRITICAL_CRASH_THRESHOLD    4       /* if we crash >4 times ... */
#define CRITICAL_CRASH_WINDOW       (4*60)  /* ... in 4 minutes, goto recovery*/
#define MINIMUM_UPTIME              (3*60)  /* reboot after system's up more than 3 min */

#ifdef BUILD_EUREKA
static void release_wake_lock(const char *name, int name_len)
{
    int fd = TEMP_FAILURE_RETRY(open("/sys/power/wake_unlock",
                                     O_WRONLY | O_CLOEXEC));
    if (fd < 0) goto out;

    int written = TEMP_FAILURE_RETRY(write(fd, name, name_len));
    if (written != name_len) {
        ERROR("Failure releasing wake lock %s\n", name);
    } else {
        INFO("Released wake lock %s\n", name);
    }

out:
    close(fd);
}

static void release_dangling_wake_locks(const char *name_prefix,
                                        int name_prefix_len)
{
    int fd = -1;
    const int max_file_size = 4096;
    char *data = NULL;

    if (!name_prefix_len) goto out;

    data = (char*) malloc(max_file_size);
    if (!data) goto out;

    fd = TEMP_FAILURE_RETRY(open("/sys/power/wake_lock", O_RDONLY | O_CLOEXEC));
    if (fd < 0) goto out;

    int read_size = TEMP_FAILURE_RETRY(read(fd, data, max_file_size-1));
    if (read_size <= 0) {
        goto out;
    } else if (read_size >= max_file_size-1) {
        ERROR("/sys/power/wake_lock is too large\n");
    }
    data[read_size-1] = 0;

    char *name_save;
    char *name = strtok_r(data, " \n", &name_save);
    while (name) {
        if (strncmp(name, name_prefix, name_prefix_len) == 0) {
            int name_len = strnlen(name, max_file_size - (name - data));
            release_wake_lock(name, name_len);
        }
        name = strtok_r(NULL, " \n", &name_save);
    }

out:
    close(fd);
    if (data) {
        free(data);
    }
}

static void release_service_wakelocks(const struct service *svc)
{
    char wake_lock_prefix[128];
    int wake_lock_prefix_length = snprintf(wake_lock_prefix,
            sizeof(wake_lock_prefix), "svc.%s.", svc->name);
    if (wake_lock_prefix_length > 0 &&
        wake_lock_prefix_length < (int)sizeof(wake_lock_prefix)) {
      release_dangling_wake_locks(wake_lock_prefix, wake_lock_prefix_length);
    } else {
      ERROR("failed to clean dangling wake locks for %s", svc->name);
    }
}
#endif

static int wait_for_one_process(int block)
{
    pid_t pid;
    int status;
    struct service *svc;
    struct socketinfo *si;
    time_t now;
    struct listnode *node;
    struct command *cmd;

    while ( (pid = waitpid(-1, &status, block ? 0 : WNOHANG)) == -1 && errno == EINTR );
    if (pid <= 0) return -1;
    INFO("waitpid returned pid %d, status = %08x\n", pid, status);

    svc = service_find_by_pid(pid);
    if (!svc) {
        ERROR("untracked pid %d exited\n", pid);
        return 0;
    }

    NOTICE("process '%s', pid %d exited\n", svc->name, pid);

    if (!(svc->flags & SVC_ONESHOT)) {
        kill(-pid, SIGKILL);
        NOTICE("process '%s' killing any children in process group\n", svc->name);
    }

    /* remove any sockets we may have created */
    for (si = svc->sockets; si; si = si->next) {
        char tmp[128];
        snprintf(tmp, sizeof(tmp), ANDROID_SOCKET_DIR"/%s", si->name);
        unlink(tmp);
    }

#ifdef BUILD_EUREKA
    release_service_wakelocks(svc);
#endif

    svc->pid = 0;
    svc->flags &= (~SVC_RUNNING);

        /* oneshot processes go into the disabled state on exit */
    if (svc->flags & SVC_ONESHOT) {
        svc->flags |= SVC_DISABLED;
    }

        /* disabled and reset processes do not get restarted automatically */
    if (svc->flags & (SVC_DISABLED | SVC_RESET) )  {
        notify_service_state(svc->name, "stopped");
        return 0;
    }

    now = gettime();
    if (svc->flags & SVC_CRITICAL) {
        if (svc->time_crashed + CRITICAL_CRASH_WINDOW >= now) {
            if (++svc->nr_crashed > CRITICAL_CRASH_THRESHOLD) {
                struct sysinfo info;
                sysinfo(&info);
                if (info.uptime >= MINIMUM_UPTIME) {
                    ERROR("critical process '%s' exited %ld times;"
                          "rebooting into recovery mode\n", svc->name,
                          svc->time_crashed);
                    android_reboot(ANDROID_RB_RESTART, 0, 0);
                    return 0;
                }
            }
        } else {
            svc->time_crashed = now;
            svc->nr_crashed = 1;
        }
    }

    svc->flags |= SVC_RESTARTING;

    /* Execute all onrestart commands for this service. */
    list_for_each(node, &svc->onrestart.commands) {
        cmd = node_to_item(node, struct command, clist);
        cmd->func(cmd->nargs, cmd->args);
    }
    notify_service_state(svc->name, "restarting");
    return 0;
}

void handle_signal(void)
{
    char tmp[32];

    /* we got a SIGCHLD - reap and restart as needed */
    read(signal_recv_fd, tmp, sizeof(tmp));
    while (!wait_for_one_process(0))
        ;
}

void signal_init(void)
{
    int s[2];

    struct sigaction act;

    act.sa_handler = sigchld_handler;
    act.sa_flags = SA_NOCLDSTOP;
#if HAVE_GLIBC
    // sa_mask is of type sigset_t, not integer.
    memset(&act.sa_mask, 0, sizeof(act.sa_mask));
#else
    act.sa_mask = 0;
#endif
    act.sa_restorer = NULL;
    sigaction(SIGCHLD, &act, 0);

    /* create a signalling mechanism for the sigchld handler */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, s) == 0) {
        signal_fd = s[0];
        signal_recv_fd = s[1];
        fcntl(s[0], F_SETFD, FD_CLOEXEC);
        fcntl(s[0], F_SETFL, O_NONBLOCK);
        fcntl(s[1], F_SETFD, FD_CLOEXEC);
        fcntl(s[1], F_SETFL, O_NONBLOCK);
    }

    handle_signal();
}

int get_signal_fd()
{
    return signal_recv_fd;
}
