/* $Header: /cvsroot/watchdog/watchdog/src/test_binary.c,v 1.3 2006/09/12 09:17:01 meskes Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <linux/limits.h>

#include "extern.h"
#include "watch_err.h"
#include "watchdog_service.h"

#if USE_SYSLOG
#include <syslog.h>
#endif

#define MORE_THAN_DEFAULT_PRIORITY (-10)

struct process
{
    char proc_name[PATH_MAX];
    pid_t pid;
    time_t time;
    struct process *next;
};

static struct process *process_head = NULL;

static void add_process (const char *name, pid_t pid)
{
    struct process *node = (struct process *) malloc (sizeof (struct process));

    snprintf(node->proc_name, sizeof(node->proc_name), "%s", name);
    node->pid = pid;
    node->time = time (NULL);
    node->next = process_head;
    process_head = node;
}

static void remove_process (pid_t pid)
{
    struct process *last, *current;
    last = NULL;
    current = process_head;
    while (current != NULL && current->pid != pid) {
        last = current;
        current = current->next;
    }
    if (current != NULL) {
        if (last == NULL)
            process_head = current->next;
        else
            last->next = current->next;
        free (current);
    }
}

/* See if any test processes have exceeded the timeout */
static int check_processes (const char *name, time_t timeout)
{
    struct process *current;
    time_t now = time (NULL);
    
    current = process_head;
    while (current != NULL) {
        if (!strcmp(current->proc_name, name) &&
	      now - current->time > timeout) {
            remove_process (current->pid);
            return (ETOOLONG);
        }
        current = current->next;
    }
    return (ENOERR);
}

/* execute test binary */
int check_bin(char *tbinary, time_t timeout, int version)
{
    pid_t child_pid;
    int result, res = 0;

    if (tbinary == NULL)
        return ENOERR;

    if (timeout > 0)
	    res = check_processes(tbinary, timeout);
    if (res == ETOOLONG) {
#if USE_SYSLOG
        syslog(LOG_ERR, "test-binary %s exceeded time limit %ld", tbinary, timeout);
#endif				/* USE_SYSLOG */
        return res;
    }

    child_pid = fork();
    if (!child_pid) {
	/*
	 * Use the default scheduler and normal priority
	 * so we don't interfere with other tasks */
	// b/138244007 Actually lets bump the priority a bit
	struct sched_param param;
	param.sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &param) == -1) {
#if USE_SYSLOG
	    syslog(LOG_ERR, "Can't set scheduler for test binary %s, err =  %d", tbinary, errno);
#endif				/* USE_SYSLOG */
	}
	if (setpriority(PRIO_PROCESS, 0, MORE_THAN_DEFAULT_PRIORITY) == -1) {
#if USE_SYSLOG
	    syslog(LOG_ERR, "Can't set priority for test binary %s, err =  %d", tbinary, errno);
#endif				/* USE_SYSLOG */
	}

	/* now start binary */
	if (version == 0) {
		execl(tbinary, tbinary, NULL);
	} else {
		execl(tbinary, tbinary, "test", NULL);
	}

	/* execl should only return in case of an error */
	/* so we return that error */
	exit(errno);
    } else if (child_pid < 0) {	/* fork failed */
	int err = errno;

 	if (errno == EAGAIN) {	/* process table full */
#if USE_SYSLOG
	    syslog(LOG_ERR, "process table is full!");
#endif				/* USE_SYSLOG */
	    return (EREBOOT);
	} else if (softboot)
	    return (err);
	else
	    return (ENOERR);
    } else {
	int ret, err;

	/* fork was okay, add child to process list */
	add_process(tbinary, child_pid);

	/* wait for child(s) to stop */
	/* but only after a short sleep */
	for (int i = 0; i < tint; ++i) {
	  if (is_reboot_requested())
	    do_shutdown(EREBOOT, true);
	  usleep(500000);
	}

	do {
	    ret = waitpid(child_pid, &result, WNOHANG);
	    err = errno;
        if (ret > 0)
            remove_process(ret);
	} while (ret > 0 && WIFEXITED(result) != 0 && WEXITSTATUS(result) == 0);

	/* check result: */
	/* ret < 0 			=> error */
	/* ret == 0			=> no more child returned, however we may already have caught the actual child */
	/* WIFEXITED(result) == 0	=> child did not exit normally but was killed by signal which was not caught */
	/* WEXITSTATUS(result) != 0	=> child returned an error code */
	if (ret > 0) {
		if (WIFEXITED(result) != 0) {
			/* if one of the scripts returns an error code just return that code */
#if USE_SYSLOG
			syslog(LOG_ERR, "test binary %s returned %d", tbinary, WEXITSTATUS(result));
#endif				/* USE_SYSLOG */
		    	return (WEXITSTATUS(result));
		} else if (WIFSIGNALED(result) != 0)  {
			/* if one of the scripts was killed return ECHKILL */
#if USE_SYSLOG
			syslog(LOG_ERR, "test binary %s was killed by uncaught signal %d", tbinary, WTERMSIG(result));
#endif				/* USE_SYSLOG */
		    	return (ECHKILL);
		}
	} else {
		/* in case there are still old childs running due to an error */
		/* log that error */
		if (ret != 0 && err != 0 && err != ECHILD) {
#if USE_SYSLOG
		    errno = err;
		    syslog(LOG_ERR, "child %d did not exit immediately (error = %d = '%m')", child_pid, err);
#else				/* USE_SYSLOG */
		    perror(progname);
#endif				/* USE_SYSLOG */
		    if (softboot)
			return (err);
		}
	}
    }
    return (ENOERR);
}
