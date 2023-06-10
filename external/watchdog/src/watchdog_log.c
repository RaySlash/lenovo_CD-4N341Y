
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if USE_SYSLOG
#include <syslog.h>
#endif                /* USE_SYSLOG */

#include "watchdog_log.h"

extern char *logdir;

#ifdef EVENT_FILENAME
static char event_log_name[256];

int watchdog_event_log_init() {
    snprintf(event_log_name, sizeof(event_log_name), "%s/%s", logdir, EVENT_FILENAME);

#if USE_SYSLOG
    // dump event log from previous session
    syslog(LOG_INFO, "check %s of previous session", event_log_name);
#endif
    FILE *fp = fopen(event_log_name, "r");
    if (fp) {
#if USE_SYSLOG
        syslog(LOG_INFO, "--------- start of event log ----------");
        while (!feof(fp)) {
            char buf[256];
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                syslog(LOG_INFO, "%s", buf);
            } else {
                break;
            }
        }
        syslog(LOG_INFO, "--------- end of event log ------------");
#endif
        fclose(fp);
    } else {
#if USE_SYSLOG
        syslog(LOG_ERR, "open %s error %s", event_log_name, strerror(errno));
#endif
    }

    unlink(event_log_name);
    return 0;
}

int watchdog_event_log(const char *fmt, ...) {
    if (!event_log_name[0]) {
#if USE_SYSLOG
        syslog(LOG_ERR, "event_log_name not initialized");
#endif
        return -1;
    }
    va_list ap;
    va_start(ap, fmt);

    FILE *fp = fopen(event_log_name, "a");
    int ret;
    if (fp) {
        ret = vfprintf(fp, fmt, ap);
        fflush(fp);
        fclose(fp);
    } else {
#if USE_SYSLOG
        syslog(LOG_ERR, "open %s error %s", event_log_name, strerror(errno));
#endif
        ret = 0;
    }

    va_end(ap);
    return ret;
}
#endif //# EVENT_FILENAME

#ifdef DUMPSTATE_FILENAME
int watchdog_dumpstate() {
    FILE *fp = popen("/chrome/dumpstate -w watchdog", "r");
    char buf[BUFSIZ], real[256];
    snprintf(real, sizeof(real), "%s/%s", logdir, DUMPSTATE_FILENAME);

    FILE *fp_out = fopen(real, "w");
    if (!fp_out) {
        pclose(fp);
#if USE_SYSLOG
        syslog(LOG_ERR, "open %s error %s", real, strerror(errno));
#endif
        return -1;
    }

    while (!feof(fp)) {
        size_t ret = fread(buf, 1, sizeof(buf), fp);
        if (ferror(fp)) {
            char err_msg[256];
            snprintf(err_msg, sizeof(err_msg), "FAILED TO WRITE dumpstate: %s",
                     strerror(errno));
            fputs(err_msg, fp_out);
            break;
        }

        fwrite(buf, 1, ret, fp_out);
    }

    pclose(fp);
    fclose(fp_out);
    sync();
#ifdef EVENT_FILENAME
    watchdog_event_log("dumpstate has been saved to %s\n", real);
#endif
    return 0;
}
#endif
