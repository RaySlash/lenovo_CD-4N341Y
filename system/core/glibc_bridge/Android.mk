# Copyright 2008 Google Inc. All Rights Reserved.

ifeq ($(TARGET_USE_SYSTEM_LIBS),true)

LOCAL_PATH := $(call my-dir)

### libglibc_bridge.a

include $(CLEAR_VARS)

LOCAL_MODULE := libglibc_bridge

# Common
LOCAL_SRC_FILES := \
    src/gettid.cpp \
    src/glibc_bridge.c \
    src/ioprio.c \
    src/strtotimeval.c \
    src/system_properties.c

LOCAL_CFLAGS += -D_GNU_SOURCE

# x86-specific
ifeq ($(TARGET_ARCH),x86)
LOCAL_SRC_FILES += \
    src/arch-x86/atomics_x86.S
endif

ifeq ($(TARGET_ARCH),arm64)
LOCAL_CFLAGS += -DARCH_ARM_64
endif

# ARM-specific
ifeq ($(TARGET_ARCH),arm)
LOCAL_SRC_FILES += \
    src/arch-arm/atomics_arm.S \
    src/arch-arm/memcmp16.S \
    src/arch-arm/cacheflush.cpp
endif

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

LOCAL_MODULE_TAGS := optional

# Disable the automatic default link to libglibc_bridge.
LOCAL_SYSTEM_SHARED_LIBRARIES :=

include $(BUILD_STATIC_LIBRARY)

### libglibc_bridge.so

include $(CLEAR_VARS)

LOCAL_MODULE := libglibc_bridge
LOCAL_MODULE_TAGS := optional

LOCAL_SYSTEM_SHARED_LIBRARIES :=

LOCAL_WHOLE_STATIC_LIBRARIES := libglibc_bridge
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_LIB)

include $(BUILD_SHARED_LIBRARY)

### libc_stubs.so

include $(CLEAR_VARS)

LOCAL_MODULE := libc_stubs

LOCAL_CFLAGS :=
LOCAL_C_INCLUDES := \
	bionic/libc/stdlib \
	bionic/libc/libc/string \
	bionic/libc/libc/stdio

LOCAL_SRC_FILES := src/stubs.c
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_LIB)
LOCAL_MODULE_TAGS := optional

# Note: Object files get linked into .so's using transform-o-to-shared-lib,
# which runs ld via PRIVATE_CXX.  Since this is a C-only library (and a weird ld
# preloaded one at that), we do not want the unnecessary linkage to libstdc++
# and libm.
#
# Sneak around this by setting LOCAL_CXX to the C compiler.
LOCAL_CXX := $(TARGET_CC)

# Also, disable the automatic default link to libglibc_bridge.
LOCAL_SYSTEM_SHARED_LIBRARIES :=

LOCAL_SHARED_LIBRARIES := liblog

include $(BUILD_SHARED_LIBRARY)

endif  # TARGET_USE_SYSTEM_LIBS
