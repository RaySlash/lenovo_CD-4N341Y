#ifndef _STRUTIL_H_
#define _STRUTIL_H_

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GLIBC__) || defined(_WIN32)

#if !HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size);
#endif

size_t strlcat(char *, const char *, size_t);
#endif
#ifdef __cplusplus
} // extern "C"
#endif

#endif  // _STRUTIL_H_
