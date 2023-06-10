#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 8192
static int copy_buf[BUF_SIZE];

static const char kDumpstateFile[] = "/data/watchdog/log/dumpstate.log";
static const char kDumpstateFileCopy[] =
    "/data/chrome/minidumps/dumpstate.log";
static const char kLockFile[] = "/data/chrome/minidumps/lockfile";
static const char kMinidumpDir[] = "/data/chrome/minidumps";
static const char kMinidumpFile[] = "/system/chrome/watchdog/wd.dmp";
static const char kWatchdogUploadFlag[] = "persist.watchdog.upload";
static const char kWatchdogReason[] = "persist.watchdog.reason";
static const char kBuildNumberProp[] = "ro.build.version.incremental";
static const char kReleaseVersionProp[] = "ro.build.version.release";

static const char kKeyStatsOptIn[] = "persist.chrome.opt_in.stats";
static const char kValueStatsOptIn[] = "1";

static void copy_file(int fd, int fd_o) {
  ssize_t bytes_read, bytes_written;
  while ((bytes_read = read(fd, copy_buf, BUF_SIZE)) > 0) {
    bytes_written = write(fd_o, copy_buf, bytes_read);
  }
}

void set_watchdog_upload_flag(int flag) {
  property_set(kWatchdogUploadFlag, (flag ? "yes" : ""));
}

void setup_watchdog_upload() {
  const int kBufSize = 1024;
  char buf[kBufSize];
  char lock_line[kBufSize];
  char value[PROPERTY_VALUE_MAX + 1];
  int fd, fd_o;
  ssize_t bytes_read;

  // Use upload flag to determine if upload is needed.
  property_get(kWatchdogUploadFlag, value, "");
  if (strcmp(value, "") == 0) {
    return;
  }

  // Do not upload if user is not opt in.
  property_get(kKeyStatsOptIn, value, "");
  if (strcmp(value, kValueStatsOptIn) != 0) {
    return;
  }

  // Fetch the directory owner id
  struct stat dir_stat;
  if (stat(kMinidumpDir, &dir_stat) != 0) {
#if USE_SYSLOG
    syslog(LOG_ERR, "Can't stat(%s) %s", kMinidumpDir, strerror(errno));
#endif
    return;
  }
  if (!(dir_stat.st_mode & S_IFDIR)) {
#if USE_SYSLOG
    syslog(LOG_ERR, "%s is not a directory", kMinidumpDir);
#endif
    return;
  }
  uid_t crash_manager_uid = dir_stat.st_uid;
  gid_t crash_manager_gid = dir_stat.st_gid;

  // Copy the dumpstate file to minidump directory
  fd = open(kDumpstateFile, O_RDONLY);
  if (fd == -1) {
#if USE_SYSLOG
    syslog(LOG_ERR, "Can't open dumpstate file %s - %s",
           kDumpstateFile, strerror(errno));
#endif
    return;
  }

  fd_o = open(kDumpstateFileCopy, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd_o == -1) {
#if USE_SYSLOG
    syslog(LOG_ERR, "Can't open dumpstate file %s - %s",
           kDumpstateFileCopy, strerror(errno));
#endif
    close(fd);
    return;
  }

  copy_file(fd, fd_o);
  close(fd);
  close(fd_o);
  // Files created by watchdog is owned by root.
  // chmod to the minidump directory owner so minidump_uploader can
  // access the dumpstate file.
  if (chown(kDumpstateFileCopy, crash_manager_uid, crash_manager_gid) != 0) {
#if USE_SYSLOG
    syslog(LOG_ERR, "Can't chown(%s) %s", kDumpstateFileCopy, strerror(errno));
#endif
    unlink(kDumpstateFileCopy);
    return;
  }

  // Create the lockfile line
  time_t dump_time;
  if (time(&dump_time) == -1) {
    snprintf(buf, kBufSize, "X-X-X X:X:X");
  } else {
    strftime(buf, kBufSize, "%Y-%m-%d %H:%M:%S", gmtime(&dump_time));
  }

  // The format is:
  // This must be consistent with
  // chromium/src/chromecast/crash/linux/dump_info.h
  //
  // {
  //   "name": <name>,
  //   "dump_time": <dump_time (kDumpTimeFormat)>,
  //   "dump": <dump>,
  //   "uptime": <uptime>,
  //   "logfile": <logfile>,
  //   "build_number": <build number>
  //   "release_version": <release version>
  //   "reason": <reason>
  // }
  static const char fmt[] =
      "{"
      "\"name\":\"watchdog_upload\","
      "\"dump_time\":\"%s\","
      "\"dump\":\"%s\","
      "\"uptime\":\"0\","
      "\"logfile\":\"%s\","
      "\"build_number\":\"%s\","
      "\"release_version\":\"%s\","
      "\"reason\":\"Watchdog reset: %s\""
      "}\n";

  char build_number[PROPERTY_VALUE_MAX];
  char release_version[PROPERTY_VALUE_MAX];
  char reason[PROPERTY_VALUE_MAX];

  property_get(kBuildNumberProp, build_number, "0");
  property_get(kReleaseVersionProp, release_version, "0");
  property_get(kWatchdogReason, reason, "0");

  snprintf(lock_line, sizeof(lock_line), fmt, buf, kMinidumpFile,
           kDumpstateFileCopy, build_number, release_version, reason);
  // Update lockfile
  fd = open(kLockFile, O_RDWR | O_APPEND, 0660);
  if (fd < 0) {
#if USE_SYSLOG
    syslog(LOG_ERR, "Failed to open %s - %s", kLockFile, strerror(errno));
#endif
    unlink(kDumpstateFileCopy);
    return;
  }
  // Acquire the file lock
  int operation_mode = LOCK_EX | LOCK_NB;
  if (flock(fd, operation_mode) < 0) {
#if USE_SYSLOG
    syslog(LOG_ERR, "Failed to lock %s - %s", kLockFile, strerror(errno));
#endif
    close(fd);
    unlink(kDumpstateFileCopy);
    return;
  }
  if (write(fd, lock_line, strlen(lock_line)) != (ssize_t)strlen(lock_line)) {
#if USE_SYSLOG
    syslog(LOG_ERR, "Failed to update %s - %s", kLockFile, strerror(errno));
#endif
    unlink(kDumpstateFileCopy);
    flock(fd, LOCK_UN);
    close(fd);
    return;
  }
  flock(fd, LOCK_UN);
  close(fd);

  // Reset upload flag.
  property_set(kWatchdogUploadFlag, "");

#if USE_SYSLOG
  syslog(LOG_INFO, "dumpstate.log transferred to %s", kMinidumpDir);
#endif
}

void set_watchdog_reason(char *reason){
  property_set(kWatchdogReason, reason);
}
