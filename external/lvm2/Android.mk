# Copyright 2016 Google Inc. All Rights Reserved.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libdm
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	libdm/datastruct/bitset.c \
	libdm/datastruct/hash.c \
	libdm/datastruct/list.c \
	libdm/libdm-common.c \
	libdm/libdm-config.c \
	libdm/libdm-deptree.c \
	libdm/libdm-file.c \
	libdm/libdm-report.c \
	libdm/libdm-stats.c \
	libdm/libdm-string.c \
	libdm/libdm-targets.c \
	libdm/libdm-timestamp.c \
	libdm/mm/dbg_malloc.c \
	libdm/mm/pool.c \
	libdm/regex/matcher.c \
	libdm/regex/parse_rx.c \
	libdm/regex/ttree.c \
	libdm/ioctl/libdm-iface.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/lib/misc \
	$(LOCAL_PATH)/lib/log \
	$(LOCAL_PATH)/libdm \
	$(LOCAL_PATH)/libdm/ioctl \
	$(LOCAL_PATH)/libdm/misc

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := dmsetup
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)

LOCAL_SRC_FILES := \
	tools/dmsetup.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/lib/misc \
	$(LOCAL_PATH)/lib/log \
	$(LOCAL_PATH)/libdm \
	$(LOCAL_PATH)/libdm/misc

LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_STATIC_LIBRARIES := libdm

include $(BUILD_EXECUTABLE)
