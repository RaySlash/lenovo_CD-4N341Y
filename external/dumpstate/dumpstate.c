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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/oom.h>

#include <cutils/properties.h>
#include <cutils/strutil.h>

#include "private/android_filesystem_config.h"

#define LOG_TAG "dumpstate"
#include <cutils/log.h>

#include "dumpstate.h"

// This is a character in a string, which is either an escaped character such as
// \" or a non quote character.
#define JSON_STR_CHAR "(\\\\.|[^\"])"

// This is a GUID value in the form of version 4 as described in RFC 4122,
// section 4.4, although can be stripped or unstripped of hyphens.
#define GUID_RE \
  "[0-9a-f]{8}-?[0-9a-f]{4}-?[0-9a-f]{4}-?[0-9a-f]{4}-?[0-9a-f]{12}"

// This is an unnamed GUID that appears as either a string entry in an array
// list, or as a dictionary key. This is checked by looking at preceding
// character. The GUID must be surrounded by quotes.
#define UNNAMED_GUID_RE "([,\\[\\{])\"" GUID_RE "\""

// Redact all unnamed GUID values. This is a bit broad, but the only easy way to
// redact sensitive values used as keys in JSON objects or array values.
//
// Named GUID values should be removed based on key name, and will not be
// automatically removed by this filter.
#define REDACT_UNNAMED_GUID " -e 's/" UNNAMED_GUID_RE "/\\1" REDACTED "/ig'"

// List of configuration entries that should be redacted in all cases when
// generating dumpstate.
#define EUREKA_CONF_REDACTED                                                  \
  "private|ssdp-udn|eureka-name|receiver-metrics-id|name|longitude|latitude|" \
  "old-device-certificate"

// List of configuration entries that should only be redacted on secure
// builds. Unsecure builds will leave these values unaffected for debugging.
#define EUREKA_CONF_REDACTED_SECURE "cloud-device-id"

static char screenshot_path[PATH_MAX] = "";

/* Don't collect logcat if this is set and this is a secure build */
/* See b/8597874 */
static int restricted = 0;

/* Don't redact the mac_address if this is set */
static int keep_mac = 0;

/* Record the reason why we are collecting dumpstate */
static char* why_dump = "user-request";

/* Return a %fullness of a given partition. Returns >100% on error */
static int percentFull(const char *partition) {
    struct statvfs stats;
    int result = statvfs(partition, &stats);
    if (result != 0) {
      printf("statvfs(%s) failed: %s", partition, strerror(errno));
      return 101;
    }
    return 100 * (stats.f_blocks - stats.f_bavail) / stats.f_blocks;
}

/* returns 1 if we should ls the /data and /cache partitions */
static int paritions_getting_full() {
  int cacheFullness = percentFull("/cache");
  int dataFullness = percentFull("/data");
  printf("--- /data is %d%% full. /cache is %d%% full. ---\n\n",
         dataFullness, cacheFullness);
  return cacheFullness > kYellowZoneCacheFullness ||
      dataFullness > kYellowZoneDataFullness;
}

char* network_redaction() {
  return keep_mac ? REDACT_SSID : REDACT_SSID_AND_MACADDR;
}

char* dmesg_redaction() {
  return keep_mac
    ? REDACT_SSID_AND_SERIAL_NUM
    : REDACT_SSID_AND_MACADDR_AND_SERIAL_NUM;
}

const char* logcat_redaction(int secure_build) {
  if (!secure_build) {
    return keep_mac ? REDACT_SSID : REDACT_SSID_AND_MACADDR_NO_QUOTES;
  } else if (!restricted) {
    return keep_mac
               ? REDACT_INFO_CONSOLE_EXCEPT_ERROR " | " REDACT_LOGCAT
               : REDACT_LOGCAT REDACT_MACADDR_RE REDACT_MACADDR_NO_COLONS_RE
               " | " REDACT_INFO_CONSOLE_EXCEPT_ERROR;
  }
  return SED "\"\"";
}

/* Dumps the previous dumpstate if it exists. */
static void dump_previous_dumpstate(time_t now) {
  if (strcmp("watchdog", why_dump) == 0) {
    return;
  }
  // If we are collecting dumpstate for any other reason than
  // a watchdog reset, also report any previous watchdog reset
  // dumpstate (also prevents recursion).
  struct stat buf;
  const char *kDumpstateLog = DUMPSTATE_LOG;

  if (stat(kDumpstateLog, &buf) == -1) {
    if (errno == ENOENT) {
      printf("NOTE: no previous watchdog reset dumpstate found.\n\n");
    } else {
      printf("ERROR: can't stat %s: %s\n\n", kDumpstateLog, strerror(errno));
    }
    return;
  }

  const time_t kDaysInSeconds = 60 * 60 * 24;
  time_t age_in_seconds = (now - buf.st_mtime);

  if (age_in_seconds >= 3 * kDaysInSeconds) {
    printf("NOTE: skipping previous watchdog reset dumpstate");
    printf(" as it is %ld days old and likely not relevant.\n\n",
           age_in_seconds / kDaysInSeconds);
    return;
  }

  printf("NOTE: previous watchdog reset occurred %ld minutes ago\n\n",
         age_in_seconds / 60);

  run_redacted_command("dumpstate from the last watchdog reset", 20,
                       "/bin/cat " DUMPSTATE_LOG, network_redaction());
}

void dump_persistent_logs(bool secure_build) {
  if (secure_build && restricted) {
    // Don't dump persistent logcats
    return;
  }
  char logcatd_running[PROPERTY_VALUE_MAX];
  property_get("logd.logpersistd.enable", logcatd_running, "");
  if (strcmp(logcatd_running, "true") == 0) {
    run_redacted_command("LOGS ON DISK", 20, "/sbin/logpersist_cat.sh /data/misc/logd",
                         logcat_redaction(secure_build));
  }
}

static void dump_cpu_stats() {
  printf("\n------ KERNEL CPUFREQ ------\n");
  char path[64];
  char title[64];
  const int kMaxCpuNumber = 6;
  for (int i = 0; i < kMaxCpuNumber; ++i) {
    snprintf(title, sizeof(title), "CPU%d", i);
    snprintf(path, sizeof(path),
             "/sys/devices/system/cpu/cpu%d/cpufreq/stats/time_in_state", i);
    if (access(path, F_OK) == 0)
      dump_file(title, path);
    else
      break;
  }
}

/* dumps the current system state to stdout */
static void dumpstate() {
    time_t now = time(NULL);
    char build[PROPERTY_VALUE_MAX], fingerprint[PROPERTY_VALUE_MAX];
    char bootloader[PROPERTY_VALUE_MAX];
    char build_type[PROPERTY_VALUE_MAX];
    char date[80];
    const int secure_build = is_secure_build();

    // TODO(b/126772092): Only dump previous dumpstate if persistent logs aren't
    // present.
    dump_previous_dumpstate(now);
    property_get("ro.build.display.id", build, "(unknown)");
    property_get("ro.build.fingerprint", fingerprint, "(unknown)");
    property_get("ro.build.type", build_type, "(unknown)");
    property_get("ro.bootloader", bootloader, "(unknown)");
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S %Z", localtime(&now));

    printf("========================================================\n");
    printf("== dumpstate: %s\n", date);
    printf("== Why: %s\n", why_dump);
    printf("========================================================\n");

    printf("\n");
    printf("Build: %s\n", build);
    printf("Build fingerprint: '%s'\n", fingerprint); /* format is important for other tools */
    printf("Bootloader: %s\n", bootloader);

    printf("Kernel:\n");
    dump_file("Version", "/proc/version");

    if (access("/tmp/gallium/launcher_dumpstate", F_OK) == 0){
        dump_file("Gallium Launcher Dumpstate","/tmp/gallium/launcher_dumpstate");
    }
    if (access("/tmp/gallium/gallium_dumpstate", F_OK) == 0){
        dump_file("Gallium Dumpstate","/tmp/gallium/gallium_dumpstate");
    }

#ifdef BOARD_HAS_DUMPSTATE
    printf("========================================================\n");
    printf("== Board\n");
    printf("========================================================\n");

    dumpstate_board();
    printf("\n");
#endif

    run_redacted_command("Command line", 10, "/bin/cat /proc/cmdline",
                         network_redaction());

    run_command("STELLAR STATE", 5, "curl", "localhost:8007/stellar/state", NULL);

    printf("env:\n");
    char **env = environ;
    while(env && *env) {
      printf("\t%s\n", *env);
      ++env;
    }
    printf("\n");

    run_command("UPTIME", 10, "uptime", NULL);
    dump_file("MEMORY INFO", "/proc/meminfo");
    // Usage of toolbox top might differ from usage of ubuntu or busybox
    // utilities, therefore call it explicitly.
    run_command("CPU INFO", 10, "toolbox", "top", "-n", "1", "-d", "1", "-m", "30", "-t", NULL);
    dump_file("VIRTUAL MEMORY STATS", "/proc/vmstat");
    dump_file("VMALLOC INFO", "/proc/vmallocinfo");
    dump_file("ZONEINFO", "/proc/zoneinfo");
    dump_file("PAGETYPEINFO", "/proc/pagetypeinfo");
    dump_file("BUDDYINFO", "/proc/buddyinfo");

    // Apply a filter against identifying information on secured builds.
    const char* fts_path = "/proc/fts";
    if (secure_build) {
      run_command("FTS", 10, "toybox grep", "-v",
                  "ro\\.\\(wifi\\.mac\\|serial\\)", fts_path, NULL);
    } else {
      dump_file("FTS", fts_path);
    }

    dump_cpu_stats();

    // Same for ps.
    run_command("PROCESSES", 10, "toolbox", "ps", "-P", NULL);
    run_command("PROCESSES AND THREADS", 10, "toolbox", "ps", "-t", "-p", "-P", NULL);
    run_command("PROCRANK", 10, "procrank", NULL);

    run_redacted_command("KERNEL LOG", 10, "dmesg", dmesg_redaction());

    for_each_pid(do_showmap, "SMAPS OF ALL PROCESSES");

    run_command("FDS OF ALL PROCESSES", 10, "sh", "-c", "ls -al -R /proc/*/fd",
                NULL);

    for_each_tid(show_wchan, "BLOCKED PROCESS WAIT-CHANNELS");

    // dump_file("EVENT LOG TAGS", "/etc/event-log-tags");
#if 0
    run_command("SYSTEM LOG", 20, "logcat", "-v", "threadtime", "-d", "*:v", NULL);
#else
    if (!(secure_build && restricted)) {
      /* TODO(maclellant): When on non-secure build we want to filter out all
         MAC addresses except the Omaha checkin info. These MAC addresses are
         the only ones in the log that are surrounded by double quotes. We
         exclude them from the filter. This is a hacky solution that will leak
         user info if
         their AP MAC address is logged with double quotes, so find better fix.
       */
      run_redacted_command("SYSTEM LOG", 20, "logcat -v threadtime -d *:v",
                           logcat_redaction(secure_build));
    }
#endif
    if (!secure_build || !restricted) {
        run_command("EVENT LOG", 20, "logcat", "-b", "events", "-v", "threadtime", "-d", "*:v", NULL);
    }

    run_command("FACTORY LIST", 20, "ls", "-al", "/factory/", NULL);
    if (!secure_build) {
        dump_file("DEVICE SERIAL", "/factory/serial.txt");
    }

    if (access("/factory/locale_list.txt", F_OK) == 0) {
        dump_file("LOCALE LIST", "/factory/locale_list.txt");
    }

    if (access("/factory/sounds/meta.txt", F_OK) == 0) {
        dump_file("SOUND FILES VERSION", "/factory/sounds/meta.txt");
    }

    if (access("/factory/hw.txt", F_OK) == 0) {
        dump_file("HW INFO", "/factory/hw.txt");
    }

    if (access("/data/chrome/tmp/", F_OK) == 0) {
      run_command("/data/chrome/tmp/ LIST", 10, "ls", "-ls",
                  "/data/chrome/tmp/", NULL);
    }

    dump_file("NETWORK DEV INFO", "/proc/net/dev");

    dump_file("NETWORK ROUTES", "/proc/net/route");
    dump_file("IPv6 ROUTES", "/proc/net/ipv6_route");

    /* The following have a tendency to get wedged when wifi drivers/fw goes belly-up. */
    if (!secure_build) {
        // Only allow reporting of our mac address for non-secure builds.
        run_command("NETWORK INTERFACES", 10, "netcfg", NULL);
        // ifconfig
        run_command("NETWORK INTERFACES", 10, "ifconfig", NULL);
        // IPv6 address contains Mac address.
        dump_file("IPv6 ADDRESSES", "/proc/net/if_inet6");
    }
#if 0
    // Never collect the mac address of the user's AP, or other devices.
    dump_file("ARP CACHE", "/proc/net/arp");
#endif

    dump_file("WIRELESS DEVICES", "/proc/net/wireless");

    if (access("/bin/wpa_cli", F_OK) == 0) {
#if 0
      run_command("WIRELESS STATUS", 10, "wpa_cli", "status", NULL);
#else
      run_redacted_command("WIRELESS STATUS", 10, "wpa_cli status",
                           network_redaction());
#endif
#if 0
      // Never collect the mac address of the user's AP.
      run_command("WIFI NETWORKS", 20,
                  "wpa_cli", "list_networks", NULL);
#endif
#if 0
      run_command("WIRELESS SCAN RESULTS", 10, "wpa_cli", "bss", "0",  NULL);
#else
      // Remove mac address & ssid of the user's AP, see above.
      run_redacted_command("WIRELESS SCAN RESULTS", 10, "wpa_cli bss 0",
                           network_redaction());
#endif
    }

    dump_file("SOCKET STATUS", "/proc/net/sockstat");
    dump_file("TCP CONNECTIONS", "/proc/net/tcp");
    dump_file("UDP CONNECTIONS", "/proc/net/udp");
    dump_file("IPv6 SOCKET STATUS", "/proc/net/sockstat6");
    dump_file("TCP6 CONNECTIONS", "/proc/net/tcp6");
    dump_file("UDP6 CONNECTIONS", "/proc/net/udp6");

    dump_file("IGMP", "/proc/net/igmp");

    dump_file("INTERRUPTS (1)", "/proc/interrupts");

    dump_file("RESOLVER", "/etc/resolv.conf");

    dump_file("DHCP LOG", "/tmp/dhcp/hooks.log");

    dump_file("INTERRUPTS (2)", "/proc/interrupts");

    run_command("Cast Receiver Build Number", 10, "toybox grep",
                "cast_receiver_build_number",
                "/chrome/cast_receiver_build_property.json", NULL);

    print_properties();

    dump_file("DISK STATS", "/proc/diskstats");

    run_command("FILESYSTEMS & FREE SPACE", 10, "df", NULL);

    run_command("Time Zone", 10, "ls", "-al", "/etc/localtime",
                "/data/share/chrome/localtime", NULL);

    if (!secure_build && paritions_getting_full()) {
        /* do a recursive ls on the /data partition but mask off
         * the domains that may have been visited in localstorage.
         * We still see the sizes, however.
         *
         * Usage of toybox sed might differ from ubuntu and busy box utilities,
         * therefore call it explicitly.
         */
        run_command(
            "LIST PARTITION:", 10, "sh", "-c",
            "ls -alR /data/ | " SED
            " 's/http.*_[0-9]\\.localstorage/http_XXXX_.localstorage/ig'");

        /* No PII in the /cache partition file names */
        run_command("PARTITION: /cache/", 20, "ls", "-alR", "/cache/", NULL);
    }

    // Expands to
    // 's/([^:\\]"(\\.|[^"])*(private|...)(\\.|[^"])*":)("(\\.|[^"])*"|[0-9\.\+-]+)/\1[REDACTED]/g'
    // If the json key contains a redacted string, the value is redacted. This
    // only works for string and number literals (not booleans, objects or
    // arrays).
    //
    // Add fields that should always be redacted to EUREKA_CONF_REDACTED.
    // Add fields that should only be redacted on secure builds to
    // EUREKA_CONF_REDACTED_SECURE.
    const char* eureka_conf_re =
        SED " -e 's/([^:\\\\]\"" JSON_STR_CHAR "*(" EUREKA_CONF_REDACTED
            "|" EUREKA_CONF_REDACTED_SECURE ")" JSON_STR_CHAR
            "*\":)(\"" JSON_STR_CHAR "*\"|[0-9\\.\\+-]+)/\\1" REDACTED "/g'";
    if (!secure_build) {
      eureka_conf_re =
          SED " -e 's/([^:\\\\]\"" JSON_STR_CHAR "*(" EUREKA_CONF_REDACTED
              ")" JSON_STR_CHAR "*\":)(\"" JSON_STR_CHAR
              "*\"|[0-9\\.\\+-]+)/\\1" REDACTED "/g'";
    }

    char eureka_conf_network_re[MAX_CMD_LEN];
    snprintf(eureka_conf_network_re, MAX_CMD_LEN, "%s | %s", eureka_conf_re,
             network_redaction());
    /* chrome-related */
    /* NOTE: exclude the private key used for protecting the wifi password. */
    /* Also exclude the udn and name of the device. */
    run_redacted_command("CHROME CONFIGURATION", 10,
                         "/bin/cat /data/chrome/.eureka.conf",
                         eureka_conf_network_re);

    /* List the contents of /cache to see if an OTA file is being downloaded. */
    run_command("CACHE PARTITION CONTENTS", 10, "ls", "-a", "-l", "/cache/",
                NULL);

    run_command("DATA PARTITION CONTENTS", 10, "ls", "-a", "-l", "/data/",
                NULL);

    run_command("HDMI REGISTERS", 10, "log_hdmi_state", NULL);

    dump_file("LAST RECOVERY LOG", "/cache/recovery/last_log");

    if (!secure_build) {
      dump_file("LAST RECOVERY LOGCAT", "/cache/recovery/last_logcat");
    } else {
      run_command("LAST RECOVERY LOGCAT", 20, "sh", "-c",
                  "cat /cache/recovery/last_logcat "
                  "| " REDACT_SSID_AND_MACADDR_NO_QUOTES,
                  NULL);
    }

    // Dump pstore which contains the dmesg of last boot.
    // Cold boot will not generate pstore.
    if (access("/sys/fs/pstore/", F_OK) == 0) {

        run_command("/sys/fs/pstore/ LIST", 10, "ls", "-al", "/sys/fs/pstore/",
                    NULL);

        if (access("/sys/fs/pstore/console-ramoops-0", F_OK) == 0) {
            dump_file("CHECK PSTORE CONSOLE 0",
                      "/sys/fs/pstore/console-ramoops-0");
        }
        if (access("/sys/fs/pstore/dmesg-ramoops-0", F_OK) == 0) {
            dump_file("CHECK PSTORE DMESG 0", "/sys/fs/pstore/dmesg-ramoops-0");
        }
        if (access("/sys/fs/pstore/dmesg-ramoops-1", F_OK) == 0) {
            dump_file("CHECK PSTORE DMESG 1", "/sys/fs/pstore/dmesg-ramoops-1");
        }
    }

    dump_persistent_logs(secure_build);

    printf("========================================================\n");
    printf("== dumpstate: done\n");
    printf("========================================================\n");
}

static void usage() {
    fprintf(stderr, "usage: dumpstate [-o file [-d] [-p] [-z]] [-s] [-q] [-r] [-w why]\n"
            "  -o: write to file (instead of stdout)\n"
            "  -d: append date to filename (requires -o)\n"
            "  -z: gzip output (requires -o)\n"
            "  -p: capture screenshot to filename.png (requires -o)\n"
            "  -s: write output to control socket (for init)\n"
            "  -r: restrict dumpstate if this is a secure build (doesn't include logcat)\n"
            "  -w: why we are running dumpstate (watchdog|crash|feedback|...)\n"
		);
}

int main(int argc, char *argv[]) {
    int do_add_date = 0;
    int do_compress = 0;
    char* use_outfile = 0;
    int use_socket = 0;
    int do_fb = 0;

    // Sanitize environment variables.
    struct {
        const char* name;
        char* desired_value; // A NULL desired_value means pass through.
        bool need_free;
    } env_whitelist[] = {
        {"SECURE_USER_BUILD", NULL, false},
        {"PATH", "/bin:/usr/bin:/sbin:/xbin", false},
        {"LD_LIBRARY_PATH", "/system/vendor/lib:/system/lib:/usr/lib:/lib", false},
        {"ANDROID_PROPERTY_WORKSPACE", NULL, false},
    };

    for (size_t i = 0; i < sizeof(env_whitelist)/sizeof(env_whitelist[0]); ++i) {
        if (!env_whitelist[i].desired_value) {
            char* env_value = getenv(env_whitelist[i].name);
            if (env_value) {
                env_whitelist[i].desired_value = strdup(env_value);
                env_whitelist[i].need_free = true;
            }
        }
    }

    if (clearenv() != 0) {
        fprintf(stderr, "Failed to clear environment variables.");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < sizeof(env_whitelist)/sizeof(env_whitelist[0]); ++i) {
        if (env_whitelist[i].desired_value)
            setenv(env_whitelist[i].name, env_whitelist[i].desired_value, 1);
        if (env_whitelist[i].need_free) free(env_whitelist[i].desired_value);
    }

    /*
     * Dumpstate can be invoked as SUID program. LD_LIBRARY_PATH
     * gets reset in the glibc 2.19. In addition, LD_LIBRARY_PATH
     * is not passed to child process so run_command() will fail.
     * 2 Things need to happen:
     *  - set uid the same as effective uid.
     *  - setenv LD_LIBRARY_PATH if it is empty.
     */
    uid_t saved_uid = getuid();
    if (geteuid() != getuid()) {
      int ret = setuid(geteuid());
      if (ret) {
        fprintf(stderr, "setuid failed %s", strerror(errno));
        exit(EXIT_FAILURE);
      }
    }

    ALOGI("begin\n");

    signal(SIGPIPE, SIG_IGN);

    /* set as high priority, and protect from OOM killer */
    setpriority(PRIO_PROCESS, 0, -20);
    FILE *oom_adj = fopen("/proc/self/oom_score_adj", "w");
    if (oom_adj) {
        fprintf(oom_adj, "%d\n", OOM_SCORE_ADJ_MIN);
        fclose(oom_adj);
    }

    int c;
    while ((c = getopt(argc, argv, "dho:svzprmw:")) != -1) {
        switch (c) {
            case 'd': do_add_date = 1;       break;
            case 'o': use_outfile = optarg;  break;
            case 's': use_socket = 1;        break;
            case 'v': break;  // compatibility no-op
            case 'z': do_compress = 6;       break;
            case 'p': do_fb = 1;             break;
            case 'r': restricted = 1;        break;
            case 'm': keep_mac = 1;          break;
            case 'w': why_dump = optarg;     break;
            case '?': printf("\n");
            case 'h':
                usage();
                exit(1);
        }
    }
    ALOGI("why: %s\n", why_dump);

    char path[PATH_MAX], tmp_path[PATH_MAX];
    pid_t gzip_pid = -1;

    if (use_socket) {
        redirect_to_socket(stdout, "dumpstate");
    } else if (use_outfile) {
        strlcpy(path, use_outfile, sizeof(path));
        if (do_add_date) {
            char date[80];
            time_t now = time(NULL);
            strftime(date, sizeof(date), "-%Y-%m-%d-%H-%M-%S %Z",
                     localtime(&now));
            strlcat(path, date, sizeof(path));
        }
        if (do_fb) {
            strlcpy(screenshot_path, path, sizeof(screenshot_path));
            strlcat(screenshot_path, ".png", sizeof(screenshot_path));
        }
        strlcat(path, ".txt", sizeof(path));
        if (do_compress) strlcat(path, ".gz", sizeof(path));
        strlcpy(tmp_path, path, sizeof(tmp_path));
        strlcat(tmp_path, ".tmp", sizeof(tmp_path));
        gzip_pid = redirect_to_file(stdout, tmp_path, do_compress);
    }

    dumpstate();

    /* wait for gzip to finish, otherwise it might get killed when we exit */
    if (gzip_pid > 0) {
        fclose(stdout);
        waitpid(gzip_pid, NULL, 0);
    }

    /* rename the (now complete) .tmp file to its final location */
    if (use_outfile && rename(tmp_path, path)) {
        fprintf(stderr, "rename(%s, %s): %s\n", tmp_path, path, strerror(errno));
    }

    /* chnown the file to original user as dumpstate is setuid program */
    if (use_outfile && chown(path, saved_uid, getgid())) {
        fprintf(stderr, "chown(%s, %d, %d): %s\n", path, saved_uid, getgid(),
                strerror(errno));
    }
    ALOGI("done\n");

    return 0;
}
