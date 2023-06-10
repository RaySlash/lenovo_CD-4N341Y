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

#ifndef _DUMPSTATE_H_
#define _DUMPSTATE_H_

#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

/* Properties and constants used for file-system fullness monitoring */
/* Make sure that kYellowZone*Fullness corresponds to the same values in
 * sysmon/sysmon_properties.h
 */
#define kYellowZoneCacheFullness (40)
#define kYellowZoneDataFullness (10)

#define MAX_CMD_LEN 2048

typedef void (for_each_pid_func)(int, const char *);
typedef void (for_each_tid_func)(int, int, const char *);

/* prints the contents of a file */
int dump_file(const char *title, const char* path);

/* forks a command and waits for it to finish -- terminate args with NULL */
int run_command(const char *title, int timeout_seconds, const char *command, ...);

int run_redacted_command(const char* title, int timeout_seconds, const char* shell_cmd,
                         const char* redaction_re);

/* prints all the system properties */
void print_properties();

/* redirect output to a service control socket */
void redirect_to_socket(FILE *redirect, const char *service);

/* redirect output to a file, optionally gzipping; returns gzip pid */
pid_t redirect_to_file(FILE *redirect, char *path, int gzip_level);

/* dump Dalvik and native stack traces, return the trace file location (NULL if none) */
const char *dump_traces();

/* for each process in the system, run the specified function */
void for_each_pid(for_each_pid_func func, const char *header);

/* for each thread in the system, run the specified function */
void for_each_tid(for_each_tid_func func, const char *header);

/* Displays a blocked processes in-kernel wait channel */
void show_wchan(int pid, int tid, const char *name);

/* Runs "showmap" for a process */
void do_showmap(int pid, const char *name);

/* Gets the dmesg output for the kernel */
void do_dmesg();

/* Play a sound via Stagefright */
void play_sound(const char* path);

/* Implemented by libdumpstate_board to dump board-specific info */
void dumpstate_board();

/* NOTE: when creating new regex, be aware that special groups like \w, \s,
   and \d do not work with this version of sed. */

/* RE to match a hex digit */
#define HEX_RE "[0-9a-f]"

/* RE to match a word, equivalent of [\w-] */
#define WORD_RE "[0-9a-z_-]"

/* RE to match a mac address with lower case hex digits */
#define MAC_ADDRESS_RE "(" HEX_RE HEX_RE ":){5}" HEX_RE HEX_RE

/* RE to match a mac address with lower case hex digits that is not
   surrounded by double quotes ("") */
#define MAC_ADDRESS_NO_QUOTES_RE "(^|[^\"])" MAC_ADDRESS_RE "($|[^\"])"

#define REDACTED "[REDACTED]"
#define DUMPSTATE_LOG "/data/watchdog/log/dumpstate.log"
#define REDACT_MACADDR_RE " -e 's/" MAC_ADDRESS_RE "/" REDACTED "/ig'"
#define REDACT_MACADDR_NO_QUOTES_RE         \
  " -e 's/" MAC_ADDRESS_NO_QUOTES_RE "/" REDACTED "/ig'"
#define REDACT_SSID_RE " -e 's/ssid.*$/ssid" REDACTED "/ig'"
#define REDACT_HOSTAPD_SSID_RE " -e 's/\\/tmp\\/band-steering\\/s_.*$/"    \
  "\\/tmp\\/band-steering\\/s_" REDACTED "/ig'"

/* The RE below will match all hex strings longer than 12 characters. This will
   be applied to all cast logs. */
#define REDACT_HEX_STRING_RE                                               \
  " -e 's/" HEX_RE "{12}" HEX_RE "+/" REDACTED "/ig'"

/* The following RE is ported from gwifi text santizer found at:
   cs/chromeos_internal/src/platform/ap/common/text_sanitizer.cc

   These redactions will only occur on lines containing gwifi daemon names. */
#define GWIFI_RE "/ ap[a-z _-]+:| webservd:| speed-test:/"
#define REDACT_GCM_IID_TOKEN_RE                                            \
  " -e '" GWIFI_RE "s/" WORD_RE "{11}:" WORD_RE "{140}/" REDACTED "/ig'"
#define REDACT_MACADDR_NO_COLONS_RE                                        \
  " -e '" GWIFI_RE "s/" HEX_RE "{12}/" REDACTED "/ig'"
#define REDACT_IPV6_ADDR_RE                                                \
  " -e '" GWIFI_RE "s/(" HEX_RE "{1,4}:){7}"                               \
  HEX_RE "{1,4}\\/?\\d{0,3}/" REDACTED "/ig'"
#define REDACT_IPV6_ADDR2_RE                                               \
  " -e '" GWIFI_RE "s/(" HEX_RE "{1,4}:)+:(" HEX_RE "{1,4}:?)*\\/?"        \
  "\\d{0,3}/" REDACTED "/ig'"
#define REDACT_URLS_RE                                                     \
  " -e '" GWIFI_RE "s/https?:\\/\\/[^\"'\"'\"',;!<> ]*/" REDACTED "/ig'"
#define REDACT_EMAILS_RE                                                   \
  " -e '" GWIFI_RE "s/[^@\"'\"'\"',;!<> ]*@[^@\"'\"'\"',;!<> ]*/" REDACTED \
  "/ig'"
#define REDACT_UUID_RE " -e 's/" HEX_RE "{8}[-_]" HEX_RE "{4}[-_]" HEX_RE  \
  "{4}[-_]" HEX_RE "{4}[-_]" HEX_RE "{12}/[REDACTED]/ig'"

#define REMOVE_XML_REQUEST " -e '/XMLHttpRequest/d'"
#define REMOVE_VERBOSE " -e '/VERBOSE[123]:/d'"
#define REMOVE_FRIENDLY_NAME " -e '/friendly-\?name/d'"
#define REMOVE_SERIAL_NUM " -e '/serial_num=/d'" " -e '/SerialNumber/d'"

#define SED "toybox sed -r "
#define REDACT_MACADDR SED REDACT_MACADDR_RE
#define REDACT_SSID SED REDACT_SSID_RE
#define REDACT_SSID_AND_MACADDR SED REDACT_SSID_RE REDACT_MACADDR_RE
#define REDACT_SSID_AND_SERIAL_NUM \
  SED REDACT_SSID_RE REMOVE_SERIAL_NUM
#define REDACT_SSID_AND_MACADDR_AND_SERIAL_NUM \
  SED REDACT_SSID_RE REDACT_MACADDR_RE REMOVE_SERIAL_NUM
#define REDACT_SSID_AND_MACADDR_NO_QUOTES               \
  SED REDACT_SSID_RE REDACT_MACADDR_NO_QUOTES_RE

#define REDACT_LOGCAT                           \
  SED REDACT_SSID_RE                            \
  REDACT_HOSTAPD_SSID_RE                        \
  REDACT_GCM_IID_TOKEN_RE                       \
  REDACT_HEX_STRING_RE                          \
  REDACT_IPV6_ADDR_RE                           \
  REDACT_IPV6_ADDR2_RE                          \
  REDACT_URLS_RE                                \
  REDACT_EMAILS_RE                              \
  REDACT_UUID_RE                                \
  REMOVE_XML_REQUEST                            \
  REMOVE_VERBOSE                                \
  REMOVE_FRIENDLY_NAME                          \
  REMOVE_SERIAL_NUM

#define REDACT_INFO_CONSOLE_EXCEPT_ERROR \
  SED "'/INFO:CONSOLE.*[Ee][Rr][Rr][Oo][Rr]/p;" \
  "/INFO:CONSOLE/c\\*** INFO:CONSOLE log redacted ***'"

#endif /* _DUMPSTATE_H_ */
