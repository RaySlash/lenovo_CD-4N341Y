# Copyright 2006-2014 The Android Open Source Project

LOCAL_PATH := $(call my-dir)

cflags = \
     -Wall \
     -Wextra \
     -Werror

logcat_libs = \
    libbase \
    libcutils \
    liblog \
    libpcrecpp

include $(CLEAR_VARS)
LOCAL_MODULE := liblogcat
LOCAL_SRC_FILES := \
    logcat.cpp \
    getopt_long.cpp \
    logcat_system.cpp

LOCAL_CFLAGS += $(cflags)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_SHARED_LIBRARIES := $(logcat_libs)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := logcat
LOCAL_SRC_FILES := \
    logcat_main.cpp
LOCAL_SHARED_LIBRARIES := liblogcat $(logcat_libs)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(cflags)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := logcatd
LOCAL_SRC_FILES := \
    logcatd_main.cpp
LOCAL_SHARED_LIBRARIES := liblogcat $(logcat_libs)
LOCAL_CFLAGS += $(cflags)
include $(BUILD_EXECUTABLE)

include $(call first-makefiles-under,$(LOCAL_PATH))
