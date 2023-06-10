#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <string.h>
#include <unistd.h>

#if USE_SYSLOG
#include <syslog.h>
#endif

#include <cutils/sockets.h>

#include "extern.h"
#include "watch_err.h"
#include "watchdog_service.h"
#include "command_listener.h"

#if defined(BUILD_EUREKA)
#include "watchdog_upload_util.h"
#endif

static bool reboot_requested = false;

// initialize reoboot service
int reboot_service_init() {
#if USE_SYSLOG
    syslog(LOG_INFO, "start reboot service listener");
#endif

    CommandListener *cl = new CommandListener();

    if (!cl || cl->startListener()) {
#if USE_SYSLOG
        syslog(LOG_ERR, "Unable to start CommandListener (%s)", strerror(errno));
#endif
        return -1;
    }

    return 0;
}

int reboot_service_request() {
#if USE_SYSLOG
    syslog(LOG_INFO, "request reboot service");
#endif
#if defined(BUILD_EUREKA)
    set_watchdog_reason(const_cast<char*>("reboot-service"));
#endif
    reboot_requested = true;
    return 0;
}

bool is_reboot_requested() {
    return reboot_requested;
}
