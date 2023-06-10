# Copyright (C) 2012 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and

LOCAL_PATH := $(call my-dir)

# libmetrics.so
include $(CLEAR_VARS)
LOCAL_MODULE := libmetrics
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
  c_metrics_library.cc \
  metrics_library.cc
LOCAL_CPP_EXTENSION := .cc
LOCAL_C_INCLUDES := external/gtest/include
LOCAL_CFLAGS += -Wall -Werror
LOCAL_CPPFLAGS := -std=gnu++11
# Copy the headers to the build sysroot directory as mesh SDK uses libmetrics
# directly.
include_dir := $(LOCAL_PATH)
include_files := $(shell find $(include_dir) -type f -name '*.h')
LOCAL_TOOLCHAIN_PREBUILTS += \
	$(join $(include_files),\
	$(patsubst $(include_dir)/%,:usr/include/metrics/%,$(include_files)))
ifeq ($(TARGET_BUILD_VARIANT),user)
LOCAL_CFLAGS += -DEUREKA_BUILD_USER
endif
include $(BUILD_SHARED_LIBRARY)

# metrics_client
include $(CLEAR_VARS)
LOCAL_MODULE := metrics_client
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := metrics_client.cc
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS += -Wall -Werror
LOCAL_CPPFLAGS := -std=gnu++11
LOCAL_C_INCLUDES := external/gtest/include
LOCAL_SHARED_LIBRARIES := libmetrics
include $(BUILD_EXECUTABLE)
