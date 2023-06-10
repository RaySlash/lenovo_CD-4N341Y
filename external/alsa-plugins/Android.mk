LOCAL_PATH := $(call my-dir)

SUB_MAKEFILES := $(call all-subdir-makefiles)
COMMON_ALSA_PLUGINS_INCLUDES := $(call include-path-for, libasound)
COMMON_ALSA_PLUGINS_CFLAGS := -D_GNU_SOURCE
COMMON_ALSA_PLUGINS_LIBS := asound

include $(SUB_MAKEFILES)
