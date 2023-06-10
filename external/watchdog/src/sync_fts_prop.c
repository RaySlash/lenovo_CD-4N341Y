#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#if USE_SYSLOG
#include <syslog.h>
#endif
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "extern.h"
#include "watch_err.h"

#include <cutils/properties.h>
#include <flash_ts.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

// Table of boolean system properties which will be synced with fts.
static const char *kKeyProperty[] = {
    // property recording first activation date.
    "persist.chrome.activation_date",
    // property recording device UMA client id, not linked with serial number.
    "persist.chrome.client_id",
    // property recording total number of FDR events to date.
    "persist.chrome.fdr_count",
    // property recording total number of FDR events to date, but incremented
    // in fts.
    "persist.fdr_watchdog",
    // property recording user opt-out of sending usage report. Keep the name
    // as "opt_in" for backward-compatibility, see "usage_report" below.
    "persist.chrome.opt_in.stats",
    // property recording whether omaha checkin has occurred.
    "persist.omaha.checkin",
    // property for device color set in factory.
    "persist.color",
    // property for country code set in factory.
    "ro.country",
    // property for language tag set in factory.
    "ro.lang",
    // property set to 'true' if supports low vcore. 'false' otherwise.
    "ro.low_vcore_enabled",
};

// Matching fts keys for above properties.
static const char *kKeyFTS[] = {
    "activation_date",
    "client_id",
    "fdr_count",
    "fdr_watchdog",
    // usage_report should be interpreted as opt-out, i.e.
    // usage_report=0 means user opt out of sending usage_report;
    // usage_report=1 means user did not opt out.
    "usage_report",
    "omaha_checkin",
    "color",
    "ro.country",
    "ro.lang",
    "ro.low_vcore_enabled",
};

static int open_fts() {
    int fd = open(FTS_DEVICE, O_RDWR);
    if (fd == -1) {
        // Only print out the warning once to avoid flooding log.
        static int warned = 0;
        if (warned) return -1;
        warned = 1;

#if USE_SYSLOG
        syslog(LOG_ERR,
#else
        fprintf(stderr,
#endif
               "error open %s: %s\n", FTS_DEVICE, strerror(errno));
    }
    return fd;
}

// Helper to read FTS property.
static int read_fts_property(const int fd, struct flash_ts_io_req *req,
                             const char *key) {
    if (!req || !key || fd < 0) return -1;
    memset(req, 0, sizeof(*req));
    snprintf(req->key, sizeof(req->key), "%s", key);
    if (ioctl(fd, FLASH_TS_IO_GET, req)) {
#if USE_SYSLOG
        syslog(LOG_ERR,
#else
        fprintf(stderr,
#endif
               "ioctl FLASH_TS_IO_GET fail %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static int write_fts_property(const int fd, const char *key, const char *value) {
    if (!key || !value || fd < 0) return -1;

    struct flash_ts_io_req req;
    memset(&req, 0, sizeof(req));
    snprintf(req.key, sizeof(req.key), "%s", key);
    snprintf(req.val, sizeof(req.val), "%s", value);
    if (ioctl(fd, FLASH_TS_IO_SET, &req)) {
#if USE_SYSLOG
        syslog(LOG_ERR,
#else
        fprintf(stderr,
#endif
               "ioctl FLASH_TS_IO_SET fail %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

// Sync information between system properties and fts.
//
// The fts_to_prop parameter indicates the direction.
//
// We sync from fts to properties only at boot.
//
// We sync from properties to fts periodically so that
// the properties become persistent across FDRs.
//
// Some properties (e.g. ro.*) are only sync'ed one-way.
static void sync_properties_fts(int fts_to_prop) {
    // open fts
    int fd = open_fts();
    if (fd < 0) return;

    for (size_t index = 0; index < sizeof(kKeyProperty) / sizeof(*kKeyProperty);
         ++index) {
        int ro_prop = (strncmp("ro.", kKeyProperty[index], 3) == 0);
        // Only copy read-only fts values to properties, not the other way.
        // This means that these properties can only be set by changing the
        // FTS.
        //
        // Note: we could change this behavior if we wanted this
        // functionality.
        if ((ro_prop || !strcmp(kKeyFTS[index], "fdr_watchdog")) &&
            !fts_to_prop) {
            continue;
        }

        // read fts
        struct flash_ts_io_req req;
        if (read_fts_property(fd, &req, kKeyFTS[index]) == -1) {
            close(fd);
            return;
        }

        // read property
        char value[PROPERTY_VALUE_MAX];
        property_get(kKeyProperty[index], value, "");

        // If values are identical, there is nothing to do.
        if (strncmp(value, req.val, min(sizeof(req.val), PROPERTY_VALUE_MAX)) ==
            0) {
            continue;
        }

        // Sync them based on direction and values.
        // We always want to overwrite the FDR property.
        if (fts_to_prop &&
            (*value == '\0' || !strcmp(kKeyFTS[index], "fdr_watchdog"))) {
            // We are running the one-time initialization script.
            //
            // If a property is unset (and therefore the fts must be set)
            // then initialize the property to the fts value.
            //
            // If a property is set (and our fts hasn't been set yet),
            // then do nothing.  The next sync after booting will remedy
            // this
            // discrepancy.  This could happen if we reboot after
            // the property is set, but before the fts is synced.
            property_set(kKeyProperty[index], req.val);
        } else if (!fts_to_prop && *value != '\0') {
            // We are running the periodic sync process.
            //
            // If a property is set (and the fts value is different
            // or unset), set fts to the current property value.
            //
            // Note: we don't support clearing the fts
            // (accidentally or intentionally) as these properties
            // are intended to be sticky.  For example, removing
            // /data/properties/* or doing a setprop with a null
            // value won't change fts.  This doesn't normally occur
            // and the value will be reset at next boot.
            if (write_fts_property(fd, kKeyFTS[index], value) == -1) {
                close(fd);
                return;
            }
        }
    }

    close(fd);
}

// helper functions which sync in each direction.
void copy_fts_to_properties() {
  sync_properties_fts(1);
}

void copy_properties_to_fts() {
  sync_properties_fts(0);
}

void increment_fdr_count() {
    // Only increment once maximum per session, to avoid edge cases such as the
    // app + button causing fdr at the same time and the count incrementing
    // twice.
    static int fdr_incremented = 0;
    if (fdr_incremented) return;
    ++fdr_incremented;

    int fd = open_fts();
    if (fd < 0) {
        return;
    }

    struct flash_ts_io_req req;
    if (read_fts_property(fd, &req, "fdr_watchdog") == -1) {
        close(fd);
        return;
    }

    int fdr_count = strtol(req.val, NULL, 10);

    char fdr_count_str[FLASH_TS_MAX_VAL_SIZE];
    sprintf(fdr_count_str, "%d", fdr_count + 1);
    write_fts_property(fd, req.key, fdr_count_str);
    close(fd);
}
