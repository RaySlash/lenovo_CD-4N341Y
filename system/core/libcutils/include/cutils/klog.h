/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef _CUTILS_KLOG_H_
#define _CUTILS_KLOG_H_

#include <sys/cdefs.h>
#include <sys/uio.h>
#include <stdarg.h>

__BEGIN_DECLS

void klog_set_level(int level);
void klog_write(int level, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));
void klog_writev(int level, const struct iovec* iov, int iov_count);

__END_DECLS


#define KLOG_CLOSE         0
#define KLOG_OPEN          1
#define KLOG_READ          2
#define KLOG_READ_ALL      3
#define KLOG_READ_CLEAR    4
#define KLOG_CLEAR         5
#define KLOG_CONSOLE_OFF   6
#define KLOG_CONSOLE_ON    7
#define KLOG_CONSOLE_LEVEL 8
#define KLOG_SIZE_UNREAD   9
#define KLOG_SIZE_BUFFER   10

#define KLOG_ERROR_LEVEL   3
#define KLOG_WARNING_LEVEL 4
#define KLOG_NOTICE_LEVEL  5
#define KLOG_INFO_LEVEL    6
#define KLOG_DEBUG_LEVEL   7

#define KLOG_ERROR(tag,x...)   klog_write(KLOG_ERROR_LEVEL, "<3>" tag ": " x)
#define KLOG_WARNING(tag,x...) klog_write(KLOG_WARNING_LEVEL, "<4>" tag ": " x)
#define KLOG_NOTICE(tag,x...)  klog_write(KLOG_NOTICE_LEVEL, "<5>" tag ": " x)
#define KLOG_INFO(tag,x...)    klog_write(KLOG_INFO_LEVEL, "<6>" tag ": " x)
#define KLOG_DEBUG(tag,x...)   klog_write(KLOG_DEBUG_LEVEL, "<7>" tag ": " x)

#endif
