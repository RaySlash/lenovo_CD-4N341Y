#ifndef _WATCHDOG_SERVICE_H
#define _WATCHDOG_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

// reboot service
int reboot_service_init();
int reboot_service_request();
bool is_reboot_requested();

#ifdef __cplusplus
}
#endif
#endif // _WATCHDOG_SERVICE_H
