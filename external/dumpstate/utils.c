/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/klog.h>
#include <time.h>
#include <unistd.h>

#include <cutils/log.h>  /* for is_secure_build() */
#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>

#include "dumpstate.h"

static const int64_t NANOS_PER_SEC = 1000000000;

static uint64_t nanotime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * NANOS_PER_SEC + ts.tv_nsec;
}

static void __for_each_pid(void (*helper)(int, const char *, void *), const char *header, void *arg) {
    DIR *d;
    struct dirent *de;

    if (!(d = opendir("/proc"))) {
        printf("Failed to open /proc (%s)\n", strerror(errno));
        return;
    }

    printf("\n------ %s ------\n", header);
    while ((de = readdir(d))) {
        int pid;
        int fd;
        char cmdpath[255];
        char cmdline[255];

        if (!(pid = atoi(de->d_name))) {
            continue;
        }

        snprintf(cmdpath, sizeof(cmdpath), "/proc/%d/cmdline", pid);
        cmdpath[sizeof(cmdpath) - 1] = 0;
        memset(cmdline, 0, sizeof(cmdline));
        if ((fd = open(cmdpath, O_RDONLY)) < 0) {
            strcpy(cmdline, "N/A");
        } else {
            int rc = read(fd, cmdline, sizeof(cmdline) - 1);
            close(fd);
        }
        helper(pid, cmdline, arg);
    }

    closedir(d);
}

static void for_each_pid_helper(int pid, const char *cmdline, void *arg) {
    for_each_pid_func *func = arg;
    func(pid, cmdline);
}

void for_each_pid(for_each_pid_func func, const char *header) {
    __for_each_pid(for_each_pid_helper, header, func);
}

static void for_each_tid_helper(int pid, const char *cmdline, void *arg) {
    DIR *d;
    struct dirent *de;
    char taskpath[255];
    for_each_tid_func *func = arg;

    snprintf(taskpath, sizeof(taskpath), "/proc/%d/task", pid);
    taskpath[sizeof(taskpath) - 1] = 0;
    if (!(d = opendir(taskpath))) {
        printf("Failed to open %s (%s)\n", taskpath, strerror(errno));
        return;
    }

    func(pid, pid, cmdline);

    while ((de = readdir(d))) {
        int tid;
        int fd;
        char commpath[255];
        char comm[255];

        if (!(tid = atoi(de->d_name))) {
            continue;
        }

        if (tid == pid)
            continue;

        snprintf(commpath, sizeof(commpath), "/proc/%d/comm", tid);
        commpath[sizeof(commpath) - 1] = 0;
        memset(comm, 0, sizeof(comm));
        if ((fd = open(commpath, O_RDONLY)) < 0) {
            strcpy(comm, "N/A");
        } else {
            char *c;
            int rc = read(fd, comm, sizeof(comm) - 1);
            close(fd);

            c = strrchr(comm, '\n');
            if (c) {
                *c = '\0';
            }
        }
        func(pid, tid, comm);
    }

    closedir(d);
}

void for_each_tid(for_each_tid_func func, const char *header) {
    __for_each_pid(for_each_tid_helper, header, func);
}

void show_wchan(int pid, int tid, const char *name) {
    char path[255];
    char buffer[255];
    int fd;
    char name_buffer[255];

    memset(buffer, 0, sizeof(buffer));

    snprintf(path, sizeof(path), "/proc/%d/wchan", tid);
    path[sizeof(path) - 1] = 0;
    if ((fd = open(path, O_RDONLY)) < 0) {
        printf("Failed to open '%s' (%s)\n", path, strerror(errno));
        return;
    }

    if (read(fd, buffer, sizeof(buffer) - 1) < 0) {
        printf("Failed to read '%s' (%s)\n", path, strerror(errno));
        goto out_close;
    }

    snprintf(name_buffer, sizeof(name_buffer), "%*s%s",
             pid == tid ? 0 : 3, "", name);
    name_buffer[sizeof(name_buffer) - 1] = 0;

    printf("%-7d %-32s %s\n", tid, name_buffer, buffer);

out_close:
    close(fd);
    return;
}

void do_dmesg() {
    printf("------ KERNEL LOG (dmesg) ------\n");
    /* Get size of kernel buffer */
    int size = klogctl(10, NULL, 0);
    if (size <= 0) {
        printf("Unexpected klogctl return value: %d\n\n", size);
        return;
    }
    char *buf = (char *) malloc(size + 1);
    if (buf == NULL) {
        printf("memory allocation failed\n\n");
        return;
    }
    int retval = klogctl(3, buf, size);
    if (retval < 0) {
        printf("klogctl failure\n\n");
        free(buf);
        return;
    }
    buf[retval] = '\0';
    printf("%s\n\n", buf);
    free(buf);
    return;
}

void do_showmap(int pid, const char *name) {
    char title[255];
    char arg[255];

    snprintf(title, sizeof(title), "SHOW MAP %d (%s)", pid, name);
    title[sizeof(title) - 1] = 0;
    snprintf(arg, sizeof(arg), "/proc/%d/maps", pid);
    arg[sizeof(arg) - 1] = 0;
#ifdef BOARD_DUMPSTATE_FILTER_MAP_DEVZERO
    // TODO(b/139484290): Remove when vendor driver is fixed
    // This is intended as a temporary workaround for a vendor kernel driver
    // that allocates userspace virtual memory in a way that doesn't provide
    // the kernel with an associated file.
    run_command(title, 10, "/bin/grep", "-v", "/dev/zero (deleted)", "--", arg,
                NULL);
#else
    dump_file(title, arg);
#endif
}

/* prints the contents of a file */
int dump_file(const char *title, const char* path) {
    char buffer[32768];
    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        int err = errno;
        if (title) printf("------ %s (%s) ------\n", title, path);
        printf("*** %s: %s\n", path, strerror(err));
        if (title) printf("\n");
        return -1;
    }

    if (title) printf("------ %s (%s", title, path);

    if (title) {
        struct stat st;
        if (memcmp(path, "/proc/", 6) && memcmp(path, "/sys/", 5) && !fstat(fd, &st)) {
            char stamp[80];
            time_t mtime = st.st_mtime;
            strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", localtime(&mtime));
            printf(": %s", stamp);
        }
        printf(") ------\n");
    }

    int newline = 0;
    for (;;) {
        int ret = read(fd, buffer, sizeof(buffer));
        if (ret > 0) {
          // Skip NUL byte, and check if our buffer alrady has a newline
          if (buffer[ret - 1] == '\0') {
            newline = (ret > 1) && (buffer[ret - 2] == '\n');
            ret = fwrite(buffer, ret - 1, 1, stdout);
          } else {
            newline = (buffer[ret - 1] == '\n');
            ret = fwrite(buffer, ret, 1, stdout);
          }
        }
        if (ret <= 0) break;
    }

    close(fd);
    if (!newline) {
      printf("\n");
    }
    if (title) {
      printf("\n");
    }
    return 0;
}

/* forks a command and waits for it to finish */
int run_command(const char *title, int timeout_seconds, const char *command, ...) {
    fflush(stdout);
    uint64_t start = nanotime();
    pid_t pid = fork();

    /* handle error case */
    if (pid < 0) {
        printf("*** fork: %s\n", strerror(errno));
        return pid;
    }

    /* handle child case */
    if (pid == 0) {
        const char *args[MAX_CMD_LEN] = {command};
        size_t arg;

        va_list ap;
        va_start(ap, command);
        if (title) printf("------ %s (%s", title, command);
        for (arg = 1; arg < sizeof(args) / sizeof(args[0]); ++arg) {
            args[arg] = va_arg(ap, const char *);
            if (args[arg] == NULL) break;
            if (title) printf(" %s", args[arg]);
        }
        if (title) printf(") ------\n");
        fflush(stdout);

        execvp(command, (char**) args);
        printf("*** exec(%s): %s\n", command, strerror(errno));
        fflush(stdout);
        _exit(-1);
    }

    /* handle parent case */
    for (;;) {
        int status;
        pid_t p = waitpid(pid, &status, WNOHANG);
        float elapsed = (float) (nanotime() - start) / NANOS_PER_SEC;
        if (p == pid) {
            if (WIFSIGNALED(status)) {
                printf("*** %s: Killed by signal %d\n", command, WTERMSIG(status));
            } else if (WIFEXITED(status) && WEXITSTATUS(status) > 0) {
                printf("*** %s: Exit code %d\n", command, WEXITSTATUS(status));
            }
            if (title) printf("[%s: %.1fs elapsed]\n\n", command, elapsed);
            return status;
        }

        if (timeout_seconds && elapsed > timeout_seconds) {
            printf("*** %s: Timed out after %.1fs (killing pid %d)\n", command, elapsed, pid);
            kill(pid, SIGTERM);
            return -1;
        }

        usleep(100000);  // poll every 0.1 sec
    }
}

int run_redacted_command(const char* title, int timeout_seconds, const char* shell_cmd,
                         const char* redaction_re) {
  char full_cmd[MAX_CMD_LEN];
  snprintf(full_cmd, MAX_CMD_LEN, "%s | %s", shell_cmd, redaction_re);
  return run_command(title, timeout_seconds, "sh", "-c", full_cmd, NULL);
}

size_t num_props = 0;
static char* props[2000];

static void print_prop(const char *key, const char *name, void *user) {
    (void) user;
    if (num_props < sizeof(props) / sizeof(props[0])) {
        char buf[PROPERTY_KEY_MAX + PROPERTY_VALUE_MAX + 10];
        if (strcmp(key, "wifi.ap.serial_number") == 0 ||
            (is_secure_build() &&
             (strcmp(key, "ro.serialno") == 0 ||
              strcmp(key, "persist.service.bdroid.bdaddr") == 0))) {
          snprintf(buf, sizeof(buf), "[%s]: [DELETED]\n", key);
        } else {
          snprintf(buf, sizeof(buf), "[%s]: [%s]\n", key, name);
        }
        buf[sizeof(buf) - 1] = 0;
        props[num_props++] = strdup(buf);
    }
}

static int compare_prop(const void *a, const void *b) {
    return strcmp(*(char * const *) a, *(char * const *) b);
}

/* prints all the system properties */
void print_properties() {
    size_t i;
    num_props = 0;
    property_list(print_prop, NULL);
    qsort(&props, num_props, sizeof(props[0]), compare_prop);

    printf("------ SYSTEM PROPERTIES ------\n");
    for (i = 0; i < num_props; ++i) {
        fputs(props[i], stdout);
        free(props[i]);
    }
    printf("\n");
}

/* redirect output to a service control socket */
void redirect_to_socket(FILE *redirect, const char *service) {
    int s = android_get_control_socket(service);
    if (s < 0) {
        fprintf(stderr, "android_get_control_socket(%s): %s\n", service, strerror(errno));
        exit(1);
    }
    if (listen(s, 4) < 0) {
        fprintf(stderr, "listen(control socket): %s\n", strerror(errno));
        exit(1);
    }

    struct sockaddr addr;
    socklen_t alen = sizeof(addr);
    int fd = accept(s, &addr, &alen);
    if (fd < 0) {
        fprintf(stderr, "accept(control socket): %s\n", strerror(errno));
        exit(1);
    }

    fflush(redirect);
    dup2(fd, fileno(redirect));
    close(fd);
}

/* redirect output to a file, optionally gzipping; returns gzip pid (or -1) */
pid_t redirect_to_file(FILE *redirect, char *path, int gzip_level) {
    char *chp = path;

    /* skip initial slash */
    if (chp[0] == '/')
        chp++;

    /* create leading directories, if necessary */
    while (chp && chp[0]) {
        chp = strchr(chp, '/');
        if (chp) {
            *chp = 0;
            mkdir(path, 0775);  /* drwxrwxr-x */
            *chp++ = '/';
        }
    }

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        exit(1);
    }

    pid_t gzip_pid = -1;
    if (gzip_level > 0) {
        int fds[2];
        if (pipe(fds)) {
            fprintf(stderr, "pipe: %s\n", strerror(errno));
            exit(1);
        }

        fflush(redirect);
        fflush(stdout);

        gzip_pid = fork();
        if (gzip_pid < 0) {
            fprintf(stderr, "fork: %s\n", strerror(errno));
            exit(1);
        }

        if (gzip_pid == 0) {
            dup2(fds[0], STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);

            close(fd);
            close(fds[0]);
            close(fds[1]);

            char level[10];
            snprintf(level, sizeof(level), "-%d", gzip_level);
            level[sizeof(level) - 1] = 0;
            execlp("gzip", "gzip", level, NULL);
            fprintf(stderr, "exec(gzip): %s\n", strerror(errno));
            _exit(-1);
        }

        close(fd);
        close(fds[0]);
        fd = fds[1];
    }

    dup2(fd, fileno(redirect));
    close(fd);
    return gzip_pid;
}
