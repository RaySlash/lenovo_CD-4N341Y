/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>

#include <linux/capability.h>
#include <linux/prctl.h>

#include <netinet/in.h>
#include <netdb.h>

#include <cutils/properties.h>
#include <private/android_filesystem_config.h>

#define SNTP_DEBUG              0

/* NTP packet format */
#define REFERENCE_TIME_OFFSET   16
#define ORIGINATE_TIME_OFFSET   24
#define RECEIVE_TIME_OFFSET     32
#define TRANSMIT_TIME_OFFSET    40
#define NTP_PACKET_SIZE         48
#define NTP_MODE_CLIENT         3
#define NTP_MODE_UNICAST        4
#define NTP_MODE_BROADCAST      5
#define NTP_VERSION             3

/* NTP port */
#define NTP_PORT                123

/* NTP service */
#define NTP_SERVICE             "123"

/* timeout value for udp response - 5 sec */
#define NTP_TIMEOUT             (5*1000)

/* wait for 1 sec before retry in case of failure */
#define RETRY_DELAY             (1*1000)

/* for the 1st time, wait for 1 sec before retry in case of failure */
#define INITIAL_RETRY_DELAY     (1*1000)

/* Exponential backoff for retries */
#define RETRY_BACKOFF           (2)

#define MAX_RETRY_DELAY         (1048 * 1000)

/* update internal - 20 minutes (with jitter will be 10-20 minutes) */
#define UPDATE_INTERVAL         (20*60*1000)

/* threshold to update system time - 500 ms */
#define UPDATE_THRESHOLD        (500ll)

/* maximum time drift allowed to filter out response from malfunctioning server
 * - 20 sec */
#define MAX_TIME_DRIFT          (20ll*1000ll)

/* Reject times which are this much older than the build time */
#define MAX_BEFORE_BUILD_TIME   (60 * 60 * 1000)

#define BUF_SIZE 128

extern int capset(cap_user_header_t header, cap_user_data_t data);

static const unsigned long long OFFSET_1900_TO_1970 =
    ((365LL * 70LL) + 17LL) * 24LL * 60LL * 60LL;
static const char *default_ntp_server = "time.google.com";

static const char kPathSntpdLastSync[] = "/data/share/sntpd/last_sync";

// Gets the current build time. Return 0 on success.
static inline int get_build_time(struct timeval *tv) {
    const char kPropBuildDate[] = "ro.build.date.utc";
    char build_time_str[PROPERTY_VALUE_MAX];
    if (property_get(kPropBuildDate, build_time_str, "") <= 0) {
        fprintf(stderr, "Failed to get %s\n", kPropBuildDate);
        return -1;
    }

    errno = 0;
    unsigned long long parsed = strtoull(build_time_str, NULL, 0);
    if (errno != 0) {
        fprintf(stderr, "Failed to parse %s: %s\n", kPropBuildDate,
                strerror(errno));
        return -1;
    }

    tv->tv_sec = parsed;
    tv->tv_usec = 0;
    return 0;
}

static inline void record_localtime_str(struct timeval *tv, char *buf,
                                        size_t buf_size) {
    time_t nowtime = tv->tv_sec;
    struct tm *nowtm = localtime(&nowtime);
    size_t len = strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", nowtm);

    int fd = open(kPathSntpdLastSync, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "Failed to open %s: %s\n", kPathSntpdLastSync,
                strerror(errno));
        return;
    }
    if (write(fd, buf, len) != (ssize_t)len) {
        fprintf(stderr, "Write file error: %s\n", strerror(errno));
        close(fd);
        return;
    }
    // NOTE(gfhuang): For unknown reason, without this fchmod workaround, the
    // file permission stucks at 0600.
    if (fchmod(fd, 0644) != 0) {
        fprintf(stderr, "fchmod error: %s\n", strerror(errno));
    }
    close(fd);
}

/**
* read an unsigned 32 bit big endian number from the given offset in the buffer.
*/
static inline unsigned int read32(unsigned char *buffer, int offset)
{
    int i;
    unsigned int ret=0;

    for (i = 0; i < 4; i ++) {
        ret <<= 8;
        ret |= (unsigned int) buffer[offset+i];
    }
    return ret;
}

/**
* read the NTP time stamp at the given offset in the buffer and returns
* it as a system time (milliseconds since January 1, 1970).
*/
static inline unsigned long long read_timestamp(unsigned char *buffer,
        unsigned int offset)
{
    unsigned int seconds = read32(buffer, offset);
    unsigned int fraction = read32(buffer, offset + 4);

    return ((unsigned long long)seconds - OFFSET_1900_TO_1970) * 1000L
        + ((unsigned long long)fraction * 1000L) / (1LL<<32);
}


/**
* write system time (milliseconds since January 1, 1970) as an NTP time stamp
* at the given offset in the buffer.
*/
static inline void write_timeStamp(unsigned char *buffer, int offset, unsigned long long time)
{
    unsigned int seconds = time / 1000L;
    unsigned int milliseconds = time % 1000L;

    seconds += OFFSET_1900_TO_1970;
    // write seconds in big endian format
    buffer[offset++] = (unsigned char)(seconds >> 24);
    buffer[offset++] = (unsigned char)(seconds >> 16);
    buffer[offset++] = (unsigned char)(seconds >> 8);
    buffer[offset++] = (unsigned char)(seconds >> 0);

    long fraction = milliseconds * (1LL<<32) / 1000L;
    // write fraction in big endian format
    buffer[offset++] = (unsigned char)(fraction >> 24);
    buffer[offset++] = (unsigned char)(fraction >> 16);
    buffer[offset++] = (unsigned char)(fraction >> 8);
    buffer[offset++] = (unsigned char)(fraction >> 0);
}

#if SNTP_DEBUG
static void dump_timestamp(unsigned long long stamp, const char *name)
{
    fprintf(stdout, "%20s:\t%08x%08x\n",
            name,
            (int)(stamp>>32),
            (int)stamp);
}
#endif // SNTP_DEBUG

/* convert timeval to millisecond */
static inline unsigned long long tv_to_ms(struct timeval *tv)
{
    unsigned long long ret = (unsigned long long)
                             tv->tv_sec * 1000L
                             + (unsigned long long)tv->tv_usec / 1000LL;

    return ret;
}

/* convert millisecond to timeval */
static inline void ms_to_tv(struct timeval *tv, unsigned long long ms)
{
    tv->tv_sec = ms / 1000;
    tv->tv_usec = 1000 * (ms % 1000);
}

/* convert timeval to millisecond */
static inline unsigned long long ts_to_ms(const struct timespec *ts)
{
    return (unsigned long long)ts->tv_sec * 1000LL
            + (unsigned long long)ts->tv_nsec / 1000000LL;
}

static uint64_t random_int64() {
    uint64_t ret = random() | ((uint64_t)random() << 32);
    FILE* f = fopen("/dev/urandom", "r");
    if (!f) {
        fprintf(stderr, "Warning: Failed to read from /dev/urandom %s\n",
                strerror(errno));
        return ret;
    }
    fread(&ret, 1, sizeof(ret), f);
    fclose(f);
    return ret;
}

typedef int(*sleep_fn)(const struct timespec *req, struct timespec *rem);
static void sleep_ms_with_jitter(uint64_t time_ms, sleep_fn sleep_fn_ptr) {
    uint64_t jitter = random_int64() % (time_ms / 2);
    time_ms = (time_ms / 2) + jitter;

    fprintf(stderr, "sleeping for %" PRId64 " milliseconds\n", time_ms);
    struct timespec req = {
        .tv_sec = time_ms / 1000,
        .tv_nsec = (time_ms % 1000) * 1000000,
    };
    sleep_fn_ptr(&req, NULL /* rem */);
}

static int sntp_gettime(struct timeval *tv_upd)
{
    struct addrinfo hints;
    struct addrinfo *res = 0, *rp = 0;
    int s = -1, ret = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_socktype = SOCK_DGRAM;

    /* get addrinfo */
    s = getaddrinfo(default_ntp_server, NTP_SERVICE, &hints, &res);
    if (s || !res) {
        fprintf(stderr, "error getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    /*
     * pool.ntp.org is multihomed. Cycle through the server list until synced
     * with one of them.
     */
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
#if SNTP_DEBUG
            struct sockaddr_in *s = (struct sockaddr_in*)rp->ai_addr;
            char buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &s->sin_addr, buf, sizeof(buf));
            fprintf(stdout, "ipv4, %s\n", buf);
#endif // SNTP_DEBUG
        } else if (rp->ai_family == AF_INET6) {
#if SNTP_DEBUG
            struct sockaddr_in6 *s = (struct sockaddr_in6*)rp->ai_addr;
            char buf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &s->sin6_addr, buf, sizeof(buf));
            fprintf(stdout, "ipv6, %s\n", buf);
#endif // SNTP_DEBUG
        } else {
            continue;
        }

        /* create socket */
        int sock = socket(rp->ai_family, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == -1) {
            fprintf(stderr, "error open socket: %s\n", strerror(errno));
            continue;
        }

        /* set socket timeout */
        struct timeval tv;
        ms_to_tv(&tv, NTP_TIMEOUT);
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ) {
             fprintf(stderr, "error set socket option: %s\n",
                     strerror(errno));
             close(sock);
             continue;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0 ) {
             fprintf(stderr, "error set socket option: %s\n",
                     strerror(errno));
             close(sock);
             continue;
        }

        /* write NTP packet  */
        unsigned char send_buffer[NTP_PACKET_SIZE] = {0};
        send_buffer[0] = NTP_MODE_CLIENT | (NTP_VERSION << 3);
        if (gettimeofday(&tv, NULL)) {
            fprintf(stderr, "error gettimeofday: %s\n", strerror(errno));
            close(sock);
            continue;
        }

        /* get a monotonic request time */
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now)) {
            fprintf(stderr, "error clock_gettime: %s\n", strerror(errno));
            close(sock);
            continue;
        }

        unsigned long long request_time = tv_to_ms(&tv);
        unsigned long long real_request_time = ts_to_ms(&now);

        write_timeStamp(send_buffer, TRANSMIT_TIME_OFFSET, request_time);

        /* send socket */
        if (sendto(sock, send_buffer, NTP_PACKET_SIZE, 0,
                   (const struct sockaddr *)rp->ai_addr,
                   rp->ai_addrlen) == -1) {
            fprintf (stderr, "error in sendto: %s\n", strerror(errno));
            close(sock);
            continue;
        }

        /* receive socket */
        struct sockaddr_in recv_addr4 = {0};
        struct sockaddr_in6 recv_addr6 = {0};
        bool is_ipv4 = rp->ai_family == AF_INET;
        struct sockaddr* sock_addr = is_ipv4 ?
            (struct sockaddr*)&recv_addr4 : (struct sockaddr*)&recv_addr6;
        socklen_t sockaddr_len = is_ipv4 ?
            sizeof(recv_addr4) : sizeof(recv_addr6);

        unsigned char buffer[NTP_PACKET_SIZE] = {0};
        if (recvfrom(sock, buffer, NTP_PACKET_SIZE, 0,
                     sock_addr,
                     &sockaddr_len) != NTP_PACKET_SIZE) {
            fprintf (stderr, "error in recvfrom %s\n", strerror(errno));
            close(sock);
            continue;
        }

        close(sock);

        // https://tools.ietf.org/html/rfc4330#section-5
        // 1.  When the IP source and destination addresses are available for
        //     the client request, they should match the interchanged addresses
        //     in the server reply.
        // 2.  When the UDP source and destination ports are available for the
        //     client request, they should match the interchanged ports in the
        //     server reply.
        bool matches_sender = true;
        if (is_ipv4) {
            struct sockaddr_in *s = (struct sockaddr_in*)rp->ai_addr;
            matches_sender =
                (memcmp(&s->sin_addr, &recv_addr4.sin_addr,
                        sizeof(s->sin_addr)) == 0) &&
                (s->sin_port == recv_addr4.sin_port);

        } else {
            struct sockaddr_in6 *s = (struct sockaddr_in6*)rp->ai_addr;
            matches_sender =
                (memcmp(&s->sin6_addr, &recv_addr6.sin6_addr,
                        sizeof(s->sin6_addr)) == 0) &&
                (s->sin6_port == recv_addr6.sin6_port);
        }
        if (!matches_sender) {
            fprintf (stderr, "Recived packet doesn't match sender\n");
            continue;
        }

        // https://tools.ietf.org/html/rfc4330#section-5
        // 3.  The Originate Timestamp in the server reply should match the
        //     Transmit Timestamp used in the client request.
        //
        // We compare the buffers directly because {read,write}_timestamp do not
        // perfectly reverse each other.
        if (memcmp(send_buffer + TRANSMIT_TIME_OFFSET,
                   buffer + ORIGINATE_TIME_OFFSET, 8) != 0) {
            fprintf(stderr, "Originate response doesn't match request time\n");
            continue;
        }

        // https://tools.ietf.org/html/rfc4330#section-5
        // 4.  The server reply should be discarded if any of the LI, Stratum,
        //     or Transmit Timestamp fields is 0 or the Mode field is not 4
        //     (unicast) or 5 (broadcast).
        //
        // We don't care about LI (Leap (second) Indicator) because time within
        // 1 second is good enough for us.

        uint8_t stratum = buffer[1];
        if (stratum == 0) {
            fprintf(stderr, "Stratum is zero\n");
            continue;
        }

        bool is_transmit_zero = true;
        for (int i = 0; i < 8; ++i) {
            if (buffer[TRANSMIT_TIME_OFFSET + i] != 0) {
                is_transmit_zero = false;
                break;
            }
        }

        if (is_transmit_zero) {
            fprintf(stderr, "Transmit time is zero\n");
            continue;
        }

        uint8_t mode = buffer[0] & 0x7;
        if (mode != NTP_MODE_UNICAST && mode != NTP_MODE_BROADCAST) {
            fprintf(stderr, "Mode is not server or broadcast: %" PRIu8 "\n",
                    mode);
            continue;
        }


        /* receiveTime = originateTime + transit + skew
           responseTime = transmitTime + transit - skew
           clockOffset = ((receiveTime - originateTime) + (transmitTime
                         - responseTime))/2
                       = ((originateTime + transit + skew - originateTime) +
                         (transmitTime - (transmitTime + transit - skew)))/2
                       = ((transit + skew) + (transmitTime - transmitTime -
                         transit + skew))/2
                       = (transit + skew - transit + skew)/2
                       = (2 * skew)/2 = skew
        */
        /* get a monotonic respond time */
        if (clock_gettime(CLOCK_MONOTONIC, &now)) {
            fprintf(stderr, "error clock_gettime: %s\n", strerror(errno));
            continue;
        }

        // These must be signed values in order for the calculation to work
        // correctly. Signed 64 bits of milliseconds is still a long time.
        //
        // >>> 2**63 / (1000 * 60 * 60 * 24 * 365)
        // 292471208L
        long long response_time =
            request_time + (ts_to_ms(&now) - real_request_time);
        long long originate_time = read_timestamp(buffer,
                                                  ORIGINATE_TIME_OFFSET);
        long long receive_time = read_timestamp(buffer, RECEIVE_TIME_OFFSET);
        long long transmit_time = read_timestamp(buffer, TRANSMIT_TIME_OFFSET);

        // https://tools.ietf.org/html/rfc4330#section-5
        //    Timestamp Name          ID   When Generated
        //    ------------------------------------------------------------
        //    Originate Timestamp     T1   time request sent by client
        //    Receive Timestamp       T2   time request received by server
        //    Transmit Timestamp      T3   time reply sent by server
        //    Destination Timestamp   T4   time reply received by client
        //
        // The roundtrip delay d and system clock offset t are defined as:
        //
        //    d = (T4 - T1) - (T3 - T2)     t = ((T2 - T1) + (T3 - T4)) / 2.

        // d cannot be negative.
        long long d =
            (response_time - originate_time) - (transmit_time - receive_time);
        if (d < 0) {
            fprintf(stderr, "d is negative\n");
            continue;
        }

        long long t =
            (receive_time - originate_time) + (transmit_time - response_time);
        t /= 2;

        long long updated_time = (long long)request_time + t;
        if (updated_time < 0) {
            fprintf(stderr, "Reported time is negative\n");
            continue;
        }

        ms_to_tv(tv_upd, updated_time);

        struct timeval tv_build_time;
        if (get_build_time(&tv_build_time) == 0) {
            long long build_time_ms = tv_to_ms(&tv_build_time);
            if (updated_time + MAX_BEFORE_BUILD_TIME < build_time_ms) {
                fprintf(stderr, "Reported time is too old\n");
                continue;
            }
        }

#if SNTP_DEBUG
        int i;
        for (i =0; i<NTP_PACKET_SIZE; i++) {
            fprintf(stdout, "%02x", buffer[i]);
            if ((i&15) == 15) {
                fprintf(stdout, "\n");
            }
        }
        dump_timestamp(originate_time, "originate");
        dump_timestamp(receive_time, "receive");
        dump_timestamp(transmit_time, "transmit");
        dump_timestamp(request_time, "request");
        dump_timestamp(response_time, "response");
        printf("updated\n");
        dump_timestamp(updated_time, "update");
#endif
        ret = 0;
        break;
    }

    freeaddrinfo(res);
    return ret;
}

typedef int (*sntp_gettime_fn)(struct timeval *tv_upd);
static int try_sync_time(bool synced, sntp_gettime_fn sntp_gettime_fn_ptr) {
   struct timeval tv_upd;

   int ret = sntp_gettime_fn_ptr(&tv_upd);
   if (ret != 0) {
       return -1;
   }

   struct timeval tv_cur;
   unsigned long long ms_cur, ms_upd, ms_abs_diff;

   /* check threshold */
   if (gettimeofday(&tv_cur, NULL)) {
        fprintf(stderr, "error gettimeofday: %s\n",
                strerror(errno));
        return -1;
   }
   ms_cur = tv_to_ms(&tv_cur);
   ms_upd = tv_to_ms(&tv_upd);

   // unsigned abs diff
   if (ms_cur > ms_upd) {
       ms_abs_diff = ms_cur - ms_upd;
   } else {
       ms_abs_diff = ms_upd - ms_cur;
   }

   /*
    * always sync for the first time, since we don't have
    * battery-backed RTC.
    *
    * update system time if out-of-sync exceeds threshold.
    * rarely happens after initial sync (synced==1).
    */
   if (!synced || ms_abs_diff > UPDATE_THRESHOLD) {
       /*
        * discard spurious response from server if it diffs too much
        * with local system time.
        */
       if (synced && ms_abs_diff >= MAX_TIME_DRIFT) {
         fprintf(stderr, "discarding %ullms time drift from server. \n",
                 ms_abs_diff);
       } else if (settimeofday(&tv_upd, NULL)) {
           fprintf(stderr, "error settimeofday: %s\n",
                   strerror(errno));
           return -1;
       } else {
           char buf[BUF_SIZE];
           record_localtime_str(&tv_upd, buf, BUF_SIZE);
           fprintf(stderr, "synced at %s\n", buf);
       }
   } else {
       char buf[BUF_SIZE];
       record_localtime_str(&tv_cur, buf, BUF_SIZE);
       fprintf(stderr, "no change, already synced as %s\n", buf);
   }

   return 0;
}

typedef int(*sync_time_fn)(bool, sntp_gettime_fn);
static void do_sync(bool* synced, unsigned long long* retry_delay,
                    sync_time_fn sync_time_fn_ptr, sleep_fn sleep_fn_ptr,
                    sntp_gettime_fn sntp_gettime_fn_ptr) {
    if (sync_time_fn_ptr(*synced, sntp_gettime_fn_ptr) == 0) {
      *synced = true;
      *retry_delay = RETRY_DELAY;
      sleep_ms_with_jitter(UPDATE_INTERVAL, sleep_fn_ptr);
    } else {
      /* wait a couple of seconds before retry with exponential backoff */
      sleep_ms_with_jitter(*retry_delay, sleep_fn_ptr);
      if (*retry_delay < MAX_RETRY_DELAY) {
        *retry_delay *= RETRY_BACKOFF;
      }
    }
}

#ifdef STANDALONE
int main(int argc, char *argv[])
#else
int sntpd_main(int argc, char *argv[])
#endif
{
    (void)argc;

    struct __user_cap_header_struct header;
    struct __user_cap_data_struct cap[_LINUX_CAPABILITY_U32S_3];
    bool synced = false;

    /* only can run as kernel */
    if (getuid() != 0) {
        fprintf(stderr, "%s needs root to run\n", argv[0]);
        return -1;
    }

    /* setuid and keep capability  */
    prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);

    setgid(AID_INET);
    setuid(AID_SNTPD);

    header.version = _LINUX_CAPABILITY_VERSION_3;
    header.pid = 0;
    memset(cap, 0, sizeof(cap));
    int idx = CAP_TO_INDEX(CAP_SYS_TIME);
    cap[idx].effective = CAP_TO_MASK(CAP_SYS_TIME);
    cap[idx].permitted = CAP_TO_MASK(CAP_SYS_TIME);
    cap[idx].inheritable = 0;
    if (capset(&header, cap)) {
        fprintf(stderr, "setcap error: %s\n", strerror(errno));
        return -1;
    }

    // Don't start sntpd until time is already synced (b/79193398).
    int64_t build_date = property_get_int64("ro.build.date.utc", 0);
    time_t current_time = 0;
    if (build_date == 0) {
        fprintf(stderr, "Failed to parse build date\n");
        // Just wait a default amount of time.
        sleep(60);
    } else {
        // Default time is unix epoch, so wait until current time is greater
        // than the build date.
        while ((current_time = time(NULL)) < build_date) {
            sleep(1);
        }
    }

    printf("Starting time sync build_date=%" PRId64 ", current_time=%" PRId64
          "\n", build_date, (int64_t)current_time);

    unsigned long long retry_delay = INITIAL_RETRY_DELAY;

    while (1) {
        do_sync(&synced, &retry_delay, try_sync_time, nanosleep, sntp_gettime);
    }
    return 0;
}
