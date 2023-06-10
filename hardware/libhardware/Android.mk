# Copyright 2006 The Android Open Source Project

# Setting LOCAL_PATH will mess up all-subdir-makefiles, so do it beforehand.
SUBDIR_MAKEFILES := $(call all-named-subdir-makefiles,modules tests)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := libcutils liblog

LOCAL_INCLUDES += $(LOCAL_PATH)

LOCAL_CFLAGS  += -DQEMU_HARDWARE
QEMU_HARDWARE := true

LOCAL_SRC_FILES += hardware.c

LOCAL_MODULE:= libhardware

# Copy the headers to the build sysroot directory
include_dir := $(LOCAL_PATH)/include
include_files := $(shell find $(include_dir)/hardware -type f -name '*.h')
LOCAL_TOOLCHAIN_PREBUILTS := \
        $(join $(include_files),\
        $(patsubst $(include_dir)/%,:usr/include/%,$(include_files)))

include $(BUILD_SHARED_LIBRARY)

include $(SUBDIR_MAKEFILES)
