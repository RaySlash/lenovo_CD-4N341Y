#ifndef CRASH_UPLOAD_UTIL_H
#define CRASH_UPLOAD_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

// This function sets the system property "persist.watchdog.upload"
// before reboot.
void set_watchdog_upload_flag(int flag);

// At watchdog startup, this function checks if the system property
// "persist.watchdog.upload". If the property is set, it transfers
// the dumpstate log to minidump uploader directory for upload.
// The actual upload is handled by the generic crash_uploader service.
void setup_watchdog_upload();

// Set system property "persist.watchdog.reason"
void set_watchdog_reason(char *reason);

#ifdef __cplusplus
}
#endif

#endif  // CRASH_UPLOAD_UTIL_H
