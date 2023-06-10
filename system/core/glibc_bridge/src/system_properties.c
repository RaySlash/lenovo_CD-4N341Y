/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <sys/atomics.h>

#ifdef ARCH_ARM_64
#include "include/bionic_futex.h"
#endif

#if __cplusplus
extern "C" {
#endif

#define PROCFS_PATH_LEN_MAX (64)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static const char property_service_socket[] = "/dev/socket/" PROP_SERVICE_NAME;

static unsigned dummy_props = 0;

prop_area *__system_property_area__ = (void*) &dummy_props;

struct prop_file_info {
  int fd;
  int size;
};

static int get_property_workspace_env(struct prop_file_info* info) {
  int size = 0;
  int fd = 0;
  char* env = NULL;
  // If we have the ANDROID_PROPERTY_WORKSPACE env var, then the properties
  // file descriptor is available to us and we get the number (and size) from
  // that env var. If it's not there, then we probably don't have direct
  // access to the file descriptor, probably because we were started by a
  // kernel coredump, and so we look for a special env var with a path to the
  // file descriptor (and the size) that we can open

  if ((env = getenv("ANDROID_PROPERTY_WORKSPACE")) != NULL) {
    fd = strtol(env, NULL, 10);
    if (fd <= 0) {
      return -1;
    }
  } else if ((env = getenv("KERNEL_DUMPSTATE")) != NULL) {
    char path[PROCFS_PATH_LEN_MAX] = {0};
    char* path_end = strchr(env, ',');
    if (path_end == NULL) {
      return -1;
    }

    strncpy(path, env, MIN((path_end - env), PROCFS_PATH_LEN_MAX));
    fd = open(path,
              O_RDONLY);  // Handle remains open for the life of the process
    if (fd <= 0) {
      return -1;
    }
  } else {
    return -1;
  }

  // Grab the size
  env = strchr(env, ',');
  if (!env) {
    return -1;
  }
  size = strtol(env + 1, NULL, 10);
  if (size <= 0) {
    return -1;
  }

  info->fd = fd;
  info->size = size;
  return 0;
}

int __system_properties_init(void)
{
    prop_area *pa;
    char *env;
    struct prop_file_info info = {0};

    if (get_property_workspace_env(&info) != 0) {
      return -1;
    }

    if (__system_property_area__ != ((void*)&dummy_props)) {
      return 0;
    }

    pa = mmap(0, info.size, PROT_READ, MAP_SHARED, info.fd, 0);

    if(pa == MAP_FAILED) {
        return -1;
    }

    if((pa->magic != PROP_AREA_MAGIC) || (pa->version != PROP_AREA_VERSION)) {
      munmap(pa, info.size);
      return -1;
    }

    __system_property_area__ = pa;
    return 0;
}

const prop_info *__system_property_find_nth(unsigned n)
{
    prop_area *pa = __system_property_area__;

    if(n >= pa->count) {
        return 0;
    } else {
        return TOC_TO_INFO(pa, pa->toc[n]);
    }
}

const prop_info *__system_property_find(const char *name)
{
    prop_area *pa = __system_property_area__;
    unsigned count = pa->count;
    unsigned *toc = pa->toc;
    unsigned len = strlen(name);
    prop_info *pi;

    while(count--) {
        unsigned entry = *toc++;
        if(TOC_NAME_LEN(entry) != len) continue;

        pi = TOC_TO_INFO(pa, entry);
        if(memcmp(name, pi->name, len)) continue;

        return pi;
    }

    return 0;
}

int __system_property_read(const prop_info *pi, char *name, char *value)
{
    unsigned serial, len;

    for(;;) {
        serial = pi->serial;
        while(SERIAL_DIRTY(serial)) {
            __futex_wait((volatile void *)&pi->serial, serial, 0);
            serial = pi->serial;
        }
        len = SERIAL_VALUE_LEN(serial);
        memcpy(value, pi->value, len + 1);
        if(serial == pi->serial) {
            if(name != 0) {
                strcpy(name, pi->name);
            }
            return len;
        }
    }
}

int __system_property_get(const char *name, char *value)
{
    const prop_info *pi = __system_property_find(name);

    if(pi != 0) {
        return __system_property_read(pi, 0, value);
    } else {
        value[0] = 0;
        return 0;
    }
}


static int send_prop_msg(prop_msg *msg)
{
    struct pollfd pollfds[1];
    struct sockaddr_un addr;
    socklen_t alen;
    size_t namelen;
    int s;
    int r;
    int result = -1;

    s = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(s < 0) {
        return result;
    }

    memset(&addr, 0, sizeof(addr));
    namelen = strlen(property_service_socket);
    strlcpy(addr.sun_path, property_service_socket, sizeof addr.sun_path);
    addr.sun_family = AF_LOCAL;
    alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;

    if(TEMP_FAILURE_RETRY(connect(s, (struct sockaddr *) &addr, alen) < 0)) {
        close(s);
        return result;
    }

    r = TEMP_FAILURE_RETRY(send(s, msg, sizeof(prop_msg), 0));

    if(r == sizeof(prop_msg)) {
        // We successfully wrote to the property server but now we
        // wait for the property server to finish its work.  It
        // acknowledges its completion by closing the socket so we
        // poll here (on nothing), waiting for the socket to close.
        // If you 'adb shell setprop foo bar' you'll see the POLLHUP
        // once the socket closes.  Out of paranoia we cap our poll
        // at 250 ms.
        pollfds[0].fd = s;
        pollfds[0].events = 0;
        r = TEMP_FAILURE_RETRY(poll(pollfds, 1, 250 /* ms */));
        if (r == 1 && (pollfds[0].revents & POLLHUP) != 0) {
            result = 0;
        } else {
            // Ignore the timeout and treat it like a success anyway.
            // The init process is single-threaded and its property
            // service is sometimes slow to respond (perhaps it's off
            // starting a child process or something) and thus this
            // times out and the caller thinks it failed, even though
            // it's still getting around to it.  So we fake it here,
            // mostly for ctl.* properties, but we do try and wait 250
            // ms so callers who do read-after-write can reliably see
            // what they've written.  Most of the time.
            // TODO: fix the system properties design.
            result = 0;
        }
    }

    close(s);
    return result;
}

int __system_property_set(const char *key, const char *value)
{
    int err;
    int tries = 0;
    int update_seen = 0;
    prop_msg msg;

    if(key == 0) return -1;
    if(value == 0) value = "";
    if(strlen(key) >= PROP_NAME_MAX) return -1;
    if(strlen(value) >= PROP_VALUE_MAX) return -1;

    memset(&msg, 0, sizeof msg);
    msg.cmd = PROP_MSG_SETPROP;
    strlcpy(msg.name, key, sizeof msg.name);
    strlcpy(msg.value, value, sizeof msg.value);

    err = send_prop_msg(&msg);
    if(err < 0) {
        return err;
    }

    return 0;
}

unsigned __system_property_area_serial(void) {
  return __system_property_area__->serial;
}

unsigned __system_property_serial(const prop_info* pi) {
  return pi->serial;
}

int __system_property_wait(const prop_info *pi)
{
    unsigned n;
    if(pi == 0) {
        prop_area *pa = __system_property_area__;
        n = pa->serial;
        do {
            __futex_wait(&pa->serial, n, 0);
        } while(n == pa->serial);
    } else {
        n = pi->serial;
        do {
            __futex_wait((volatile void *)&pi->serial, n, 0);
        } while(n == pi->serial);
    }
    return 0;
}

#if __cplusplus
}
#endif
