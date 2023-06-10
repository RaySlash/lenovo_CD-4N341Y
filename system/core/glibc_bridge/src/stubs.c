/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include <grp.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <netdb.h>
#include <mntent.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <syslog.h>

#include <cutils/log.h>
#include <private/android_filesystem_config.h>

#define BIONIC_ALIGN(value, alignment) \
    (((value) + (alignment)-1) & ~((alignment)-1))

/** Thread-specific state for the stubs functions
 **/

// There's no destructor here, so need to fix the size of the tag.  For some
// reason, this doesn't compile with "const int kMaxTagLength = 32;"
#define kMaxTagLength 32

typedef struct {
    struct passwd  passwd;
    struct group   group;
    char*          group_members[2];
    char           app_name_buffer[32];
    char           group_name_buffer[32];
    char           log_tag[kMaxTagLength];
} stubs_state_t;

static stubs_state_t *gStubsState;

void __attribute__ ((constructor)) stubs_init(void) {
    gStubsState = calloc(1, sizeof *gStubsState);

    if (gStubsState != NULL) {
        gStubsState->group.gr_mem = gStubsState->group_members;

        // Cache the commandline basename as the log tag.
        int fd = open("/proc/self/cmdline", O_RDONLY);
        if (fd >= 0) {
            const int kMaxCmdlinePath = 1024;
            char tag_buffer[kMaxCmdlinePath];
            int bytes_read = read(fd, tag_buffer, kMaxCmdlinePath);
            if (bytes_read > 0) {
                int length = strnlen(tag_buffer, kMaxCmdlinePath);
                // If the commandline is too big, don't try to parse it.
                if (length < kMaxCmdlinePath) {
                    char* tag_basename = basename(tag_buffer);
                    strncpy(gStubsState->log_tag, tag_basename, kMaxTagLength);
                    gStubsState->log_tag[kMaxTagLength - 1] = 0;
                }
            }
            close(fd);
        }
    }
}

static stubs_state_t* __stubs_state(void)
{
    return gStubsState;
}

static void init_stubs_state(stubs_state_t* state)
{
    memset(state, 0, sizeof(*state));
    state->group.gr_mem = state->group_members;
}

static struct passwd*
android_iinfo_to_passwd( struct passwd *pw,
                         const struct android_id_info *iinfo )
{
    pw->pw_name   = (char*)iinfo->name;
    pw->pw_uid    = iinfo->aid;
    pw->pw_gid    = iinfo->aid;
    pw->pw_dir    = iinfo->dir ? iinfo->dir : "/";
    // GoogleTV is running some applications (i.e. dropbear) that expect
    // glibc-style behavior from calls to getpw* functions.  The pw_passwd
    // field needs to be populated, and pw_shell needs to be set to a shell
    // recognized by glibc's getusershell.
    pw->pw_shell  = "/bin/sh";
    pw->pw_passwd = "x";
    return pw;
}

static struct group*
android_iinfo_to_group( struct group *gr,
                        const struct android_id_info *iinfo )
{
    gr->gr_name   = (char*) iinfo->name;
    gr->gr_gid    = iinfo->aid;
    gr->gr_mem[0] = gr->gr_name;
    gr->gr_mem[1] = NULL;
    return gr;
}

static struct passwd *
android_id_to_passwd( struct passwd *pw, unsigned id)
{
    const struct android_id_info *iinfo = android_ids;
    unsigned n;
    for (n = 0; n < android_id_count; n++) {
        if (iinfo[n].aid == id) {
            return android_iinfo_to_passwd(pw, iinfo + n);
        }
    }
    return NULL;
}

static struct passwd*
android_name_to_passwd(struct passwd *pw, const char *name)
{
    const struct android_id_info *iinfo = android_ids;
    unsigned n;
    for (n = 0; n < android_id_count; n++) {
        if (!strcmp(iinfo[n].name, name)) {
            return android_iinfo_to_passwd(pw, iinfo + n);
        }
    }
    return NULL;
}

static struct group*
android_id_to_group( struct group *gr, unsigned id )
{
    const struct android_id_info *iinfo = android_ids;
    unsigned n;
    for (n = 0; n < android_id_count; n++) {
        if (iinfo[n].aid == id) {
            return android_iinfo_to_group(gr, iinfo + n);
        }
    }
    return NULL;
}

static struct group*
android_name_to_group( struct group *gr, const char *name )
{
    const struct android_id_info *iinfo = android_ids;
    unsigned n;
    for (n = 0; n < android_id_count; n++) {
        if (!strcmp(iinfo[n].name, name)) {
            return android_iinfo_to_group(gr, iinfo + n);
        }
    }
    return NULL;
}

struct passwd*
getpwuid(uid_t uid)
{
    stubs_state_t*  state = __stubs_state();
    struct passwd*  pw;

    if (state == NULL)
        return NULL;

    pw = &state->passwd;

    if ( android_id_to_passwd(pw, uid) != NULL )
        return pw;

    errno = ENOENT;
    return NULL;
}

struct passwd*
getpwnam(const char *login)
{
    stubs_state_t*  state = __stubs_state();

    if (state == NULL)
        return NULL;

    if (android_name_to_passwd(&state->passwd, login) != NULL)
        return &state->passwd;

    errno = ENOENT;
    return NULL;
}

int
getgrouplist (const char *user, gid_t group,
              gid_t *groups, int *ngroups)
{
    if (*ngroups < 1) {
        *ngroups = 1;
        return -1;
    }
    groups[0] = group;
    return (*ngroups = 1);
}

char*
getlogin(void)
{
    struct passwd *pw = getpwuid(getuid());

    if(pw) {
        return pw->pw_name;
    } else {
        return NULL;
    }
}

struct group*
getgrgid(gid_t gid)
{
    stubs_state_t*  state = __stubs_state();
    struct group*   gr;

    if (state == NULL)
        return NULL;

    gr = android_id_to_group(&state->group, gid);
    if (gr != NULL)
        return gr;

    errno = ENOENT;
    return NULL;
}

struct group*
getgrnam(const char *name)
{
    stubs_state_t*  state = __stubs_state();
    unsigned        id;

    if (state == NULL)
        return NULL;

    if (android_name_to_group(&state->group, name) != 0)
        return &state->group;

    errno = ENOENT;
    return NULL;
}

int
getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen,
           struct group **result)
{
    stubs_state_t* state = NULL;
    char* p = (char*)(BIONIC_ALIGN((uintptr_t)buf, sizeof(uintptr_t)));

    *result = NULL;
    if (p + sizeof(*state) > buf + buflen) {
        return ERANGE;
    }

    state = (stubs_state_t*)p;
    init_stubs_state(state);
    if (android_name_to_group(&state->group, name) != 0) {
        *grp = state->group;
        *result = grp;
        return 0;
    }

    return errno;
}

static void vsyslog_to_android(int priority, const char *format, va_list ap)
{
    stubs_state_t*  state = __stubs_state();
    int level = ANDROID_LOG_DEBUG;
    char* tag = "syslog";

    // The tag is the basename of the commandline $0.
    if (state != NULL && state->log_tag[0] != 0) {
        tag = state->log_tag;
    }

    if (priority <= LOG_ERR) {
        level = ANDROID_LOG_ERROR;
    } else if (priority <= LOG_WARNING) {
        level = ANDROID_LOG_WARN;
    } else if (priority <= LOG_INFO) {
        level = ANDROID_LOG_INFO;
    } else {
        level = ANDROID_LOG_DEBUG;
    }

    __android_log_buf_vprint(LOG_ID_SYSTEM, level, tag, format, ap);
}

void
#if __USE_FORTIFY_LEVEL > 0 && defined __extern_always_inline
__syslog_chk(int priority, int flag, const char* format, ...)
#else
syslog(int priority, const char* format, ...)
#endif
{
    va_list p;

    va_start(p, format);
    vsyslog_to_android(priority, format, p);
    va_end(p);
}

void
#if __USE_FORTIFY_LEVEL > 0 && defined __extern_always_inline
__vsyslog_chk(int priority, int flag, const char *format, va_list ap)
#else
vsyslog(int priority, const char *format, va_list ap)
#endif
{
    vsyslog_to_android(priority, format, ap);
}

