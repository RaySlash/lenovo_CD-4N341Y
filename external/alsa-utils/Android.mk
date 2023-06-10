LOCAL_PATH := $(call my-dir)

SUB_MAKEFILES := $(call all-subdir-makefiles)
COMMON_ALSA_UTILS_INCLUDES := $(LOCAL_PATH)/include \
                              $(call include-path-for, libasound)
COMMON_ALSA_UTILS_CFLAGS := -D_GNU_SOURCE
COMMON_ALSA_UTILS_LIBS := asound

include $(SUB_MAKEFILES)
