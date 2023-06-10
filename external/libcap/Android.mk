LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

libcap_files := \
  libcap/cap_alloc.c \
  libcap/cap_proc.c \
  libcap/cap_extint.c \
  libcap/cap_file.c \
  libcap/cap_flag.c \
  libcap/cap_text.c

LOCAL_SRC_FILES := $(libcap_files)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/libcap/include

LOCAL_MODULE := libcap

LOCAL_TOOLCHAIN_PREBUILTS := \
  $(LOCAL_PATH)/libcap/include/sys/capability.h:usr/include/sys/capability.h

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := setcap
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := progs/setcap.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libcap/include
LOCAL_STATIC_LIBRARIES := libcap
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := getcap
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := progs/getcap.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/libcap/include
LOCAL_STATIC_LIBRARIES := libcap
include $(BUILD_EXECUTABLE)
