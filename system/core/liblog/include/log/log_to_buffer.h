/*
 * Copyright (C) 2005-2017 The Android Open Source Project
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

#ifndef _LIBS_LOG_LOG_TO_BUFFER_H
#define _LIBS_LOG_LOG_TO_BUFFER_H

#include <android/log.h>
#include <log/log_id.h>

/*
 * Normally we strip the effects of ALOGV (VERBOSE messages),
 * LOG_FATAL and LOG_FATAL_IF (FATAL assert messages) from the
 * release builds be defining NDEBUG.  You can modify this (for
 * example with "#define LOG_NDEBUG 0" at the top of your source
 * file) to change that behavior.
 */

#ifndef LOG_NDEBUG
#ifdef NDEBUG
#define LOG_NDEBUG 1
#else
#define LOG_NDEBUG 0
#endif
#endif

/* --------------------------------------------------------------------- */

#ifndef __predict_false
#define __predict_false(exp) __builtin_expect((exp) != 0, 0)
#endif

#ifndef BUFLOGV
#define __BUFLOGV(id, ...) \
  ((void)__android_log_buf_print(id, ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#if LOG_NDEBUG
#define BLOGV(...)            \
  do {                        \
    if (0) {                  \
      __BUFLOGV(__VA_ARGS__); \
    }                         \
  } while (0)
#else
#define BLOGV(...) __BUFLOGV(__VA_ARGS__)
#endif
#endif
/*
 * Simplified macro to send a log message to a specific BUFFER using current LOG_TAG. */
#ifndef BLOGE
#define BLOGE(id, ...) \
  ((void)__android_log_buf_print(id, ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#ifndef BLOGI
#define BLOGI(id, ...) \
  ((void)__android_log_buf_print(id, ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#ifndef BLOGD
#define BLOGD(id, ...) \
  ((void)__android_log_buf_print(id, ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#endif

#ifndef BLOGW
#define BLOGW(id, ...) \
  ((void)__android_log_buf_print(id, ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#endif

#endif /* _LIBS_LOG_LOG_TO_BUFFER_H */
