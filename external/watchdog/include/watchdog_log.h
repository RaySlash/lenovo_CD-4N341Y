#ifndef _WATCHDOG_LOG_H_
#define _WATCHDOG_LOG_H_

#ifdef EVENT_FILENAME
int watchdog_event_log_init();
int watchdog_event_log(const char *fmt, ...);
#endif

#ifdef DUMPSTATE_FILENAME
int watchdog_dumpstate();
#endif

#endif // _WATCHDOG_LOG_H_
