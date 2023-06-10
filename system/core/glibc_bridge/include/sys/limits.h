#ifndef _SYS_LIMITS_H_
#define _SYS_LIMITS_H_

#include <limits.h>

// From https://android.googlesource.com/platform/bionic/+/master/libc/include/sys/limits.h
#define UID_MAX  UINT_MAX  /* max value for a uid_t */
#define GID_MAX  UINT_MAX  /* max value for a gid_t */

#define PAGE_SIZE 4096

#endif  // _SYS_LIMITS_H_
