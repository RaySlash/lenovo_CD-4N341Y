/* $Header: /cvsroot/watchdog/watchdog/src/pidfile.c,v 1.2 2006/07/31 09:39:23 meskes Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

#include "extern.h"
#include "watch_err.h"

#if USE_SYSLOG
#include <syslog.h>
#endif

#include "watchdog_log.h"

int check_pidfile(struct list *file)
{
    int fd = open(file->name, O_RDONLY), pid;
    char buf[10];
    
    if (fd == -1) {
	int err = errno;

#if USE_SYSLOG
	syslog(LOG_ERR, "cannot open %s (errno = %d = '%m')", file->name, err);
#else				/* USE_SYSLOG */
	perror(progname);
#endif				/* USE_SYSLOG */

	/* on error ENETDOWN|ENETUNREACH we react as if we're in ping mode 
	 * on ENOENT we assume that the server to be monitored has exited */
	if (softboot || err == ENETDOWN || err == ENETUNREACH || err == ENOENT )
	    return (err);
	
	return(ENOERR);
    }
    
    /* position pointer at start of file */
    if (lseek(fd, 0, SEEK_SET) < 0) {
	int err = errno;
        
#if USE_SYSLOG
	syslog(LOG_ERR, "lseek %s gave errno = %d = '%m'", file->name, err);
#else                           /* USE_SYSLOG */
        perror(progname);
#endif                          /* USE_SYSLOG */

	close(fd);
        if (softboot)
	        return (err);

        return (ENOERR);
    }

    /* just to play it safe */
    memset(buf, 0, sizeof(buf));

    /* read the line (there is only one) */
    if (read(fd, buf, sizeof(buf)) < 0) {
    	int err = errno;

#if USE_SYSLOG
        syslog(LOG_ERR, "read %s gave errno = %d = '%m'", file->name, err);
#else                           /* USE_SYSLOG */
        perror(progname);
#endif                          /* USE_SYSLOG */

	close(fd);
        if (softboot)
	        return (err);

        return (ENOERR);
    }

    /* we only care about integer values */
    pid = atoi(buf);

    if (close(fd) == -1) {
     	int err = errno;

#if USE_SYSLOG
        syslog(LOG_ERR, "could not close %s, errno = %d = '%m'", file->name, err);
#else                           /* USE_SYSLOG */
        perror(progname);
#endif                          /* USE_SYSLOG */

        if (softboot)
	        return (err);

        return (ENOERR);
    }

    if (kill (pid, 0) == -1) {
	int err = errno;

#if USE_SYSLOG
        syslog(LOG_ERR, "pinging process %d (%s) gave errno = %d = '%m'", pid, file->name, err);
#else                           /* USE_SYSLOG */
        perror(progname);
#endif                          /* USE_SYSLOG */

        watchdog_event_log("pinging process %d (%s): %s\n", pid, file->name, strerror(err));

        if (softboot || err == ESRCH)
	        return (err);

        return (ENOERR);
    }

    /*check if process becomes zombie */
    char name[256];
    FILE *fp;
    snprintf(name, sizeof(name), "/proc/%d/stat", pid);
    fp = fopen(name, "r");
    if (fp) {
        char status = 0;
        int ret = fscanf(fp, "%*s %*s %c", &status);
        if (ret == 1 && status == 'Z') {
            watchdog_event_log("zombie process %d (%s)\n", pid, file->name);
            return ESRCH;
        }
        fclose (fp);
    } else {
        watchdog_event_log("cannot open proc file %s", name);
        return ESRCH;
    }

#if USE_SYSLOG
    /* do verbose logging */
    if (verbose && logtick && ticker == 1)
	syslog(LOG_INFO, "was able to ping process %d (%s).", pid, file->name);
#endif

    return (ENOERR);
}
