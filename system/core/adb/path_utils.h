// Copyright (c) 2016 Google Inc. All rights reserved.

#ifndef ADB_PATH_UTILS_H_
#define ADB_PATH_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

bool IsAllowedPath(const char* path);
bool IsSafeCommand(const char* command);

#ifdef __cplusplus
}
#endif

#endif  // ADB_PATH_UTILS_H_
