LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := alsactl
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := alsactl.c \
                   daemon.c \
                   init_parse.c \
                   lock.c \
                   state.c \
                   utils.c
LOCAL_SHARED_LIBRARIES := libasound
LOCAL_CFLAGS := $(COMMON_ALSA_UTILS_CFLAGS)
LOCAL_C_INCLUDES := $(COMMON_ALSA_UTILS_INCLUDES)

include $(BUILD_EXECUTABLE)