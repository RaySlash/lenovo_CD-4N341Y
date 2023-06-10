#
# Copyright (C) 2008 The Android Open Source Project
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
# limitations under the License.
#
LOCAL_PATH := $(my-dir)
cflags := \
    -D__GLIBC__ \
    -Werror \
    -Wall \
    -Wextra \
    -Wno-unused-result


ifeq ($(TARGET_CPU_SMP),true)
    targetSmpFlag := -DANDROID_SMP=1
else
    targetSmpFlag := -DANDROID_SMP=0
endif
hostSmpFlag := -DANDROID_SMP=0

commonSources := \
    atomic.c.arm \
    config_utils.cpp \
    canned_fs_config.cpp \
    hashmap.cpp \
    iosched_policy.cpp \
    load_file.cpp \
    native_handle.cpp \
    open_memstream.c \
    record_stream.cpp \
    sched_policy.cpp \
    sockets.cpp \
    strdup16to8.cpp \
    strdup8to16.cpp \
    strutil.c \
    threads.cpp

commonHostSources := \
        ashmem-host.cpp

# some files must not be compiled when building against Mingw
# they correspond to features not used by our host development tools
# which are also hard or even impossible to port to native Win32
WINDOWS_HOST_ONLY :=
ifeq ($(HOST_OS),windows)
    ifeq ($(strip $(USE_CYGWIN)),)
        WINDOWS_HOST_ONLY := 1
    endif
endif
# USE_MINGW is defined when we build against Mingw on Linux
ifneq ($(strip $(USE_MINGW)),)
    WINDOWS_HOST_ONLY := 1
endif

ifeq ($(WINDOWS_HOST_ONLY),1)
    commonSources += \
        uio.c
else
commonSources += \
    android_get_control_file.cpp \
    fs.cpp \
    mspace.c \
    socket_inaddr_any_server_unix.cpp \
    socket_local_client_unix.cpp \
    socket_local_server_unix.cpp \
    socket_network_client_unix.cpp \
    sockets_unix.cpp \
    str_parms.cpp

# for mspace.c
cflags += \
    -Wno-unused-parameter \
    -Wno-unused-variable
endif

include $(CLEAR_VARS)

# Static library for host
# ========================================================
LOCAL_MODULE := libcutils
LOCAL_SRC_FILES := $(commonSources) $(commonHostSources) dlmalloc_stubs.c
LOCAL_LDLIBS := -lpthread

LOCAL_STATIC_LIBRARIES := \
    liblog \
    libfsconfig
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(cflags) $(hostSmpFlag)
include $(BUILD_HOST_STATIC_LIBRARY)


# Static library for host, 64-bit
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := lib64cutils
LOCAL_SRC_FILES := $(commonSources) $(commonHostSources) dlmalloc_stubs.c
LOCAL_LDLIBS := -lpthread

LOCAL_STATIC_LIBRARIES := \
    lib64log \
    libfsconfig
LOCAL_CFLAGS += $(cflags) $(hostSmpFlag) -m64
include $(BUILD_HOST_STATIC_LIBRARY)


# Shared and static library for target
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libcutils

LOCAL_SRC_FILES += \
    $(commonSources) \
    android_reboot.cpp \
    ashmem-dev.cpp \
    klog.cpp \
    partition_utils.cpp \
    properties.cpp \
    qtaguid.cpp \
    trace-dev.cpp \
    uevent.cpp

ifeq ($(TARGET_USE_SYSTEM_LIBS),true)
LOCAL_SRC_FILES += \
    dlmalloc_stubs.c
endif

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -Wno-inline-asm
LOCAL_SRC_FILES += arch-arm/memset32.S
else # !arm
ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES += arch-arm64/android_memset.S
else # !arm64
ifeq ($(TARGET_ARCH),sh)
LOCAL_SRC_FILES += memory.c atomic-android-sh.c
else  # !sh
ifeq ($(TARGET_ARCH_VARIANT),x86-atom)
LOCAL_CFLAGS += -DHAVE_MEMSET16 -DHAVE_MEMSET32
LOCAL_SRC_FILES += arch-x86/android_memset16.S arch-x86/android_memset32.S memory.c
else # !x86-atom
LOCAL_SRC_FILES += memory.c
endif # !x86-atom
endif # !sh
endif # !arm
endif # !arm64

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_STATIC_LIBRARIES := \
    liblog \
    libglibc_bridge \
    libfsconfig
LOCAL_CFLAGS += $(cflags) $(targetSmpFlag)
LOCAL_CFLAGS += -Wno-null-pointer-arithmetic
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcutils
LOCAL_WHOLE_STATIC_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(cflags) $(targetSmpFlag)
LOCAL_CFLAGS += -Wno-null-pointer-arithmetic
LOCAL_TOOLCHAIN_PREBUILTS := \
    $(LOCAL_PATH)/../include/cutils/properties.h:usr/include/cutils/properties.h \
    $(LOCAL_PATH)/../include/cutils/sockets.h:usr/include/cutils/sockets.h
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := tst_str_parms
LOCAL_CFLAGS += -DTEST_STR_PARMS
LOCAL_SRC_FILES := str_parms.c hashmap.c memory.c
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := libfsconfig
LOCAL_SRC_FILES := fs_config.cpp
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
include $(BUILD_HOST_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfsconfig
LOCAL_SRC_FILES := fs_config.cpp
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
include $(BUILD_STATIC_LIBRARY)
