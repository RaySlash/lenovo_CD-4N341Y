LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := alsaloop
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := alsaloop.c \
                   control.c \
                   pcmjob.c
LOCAL_SHARED_LIBRARIES := libasound
LOCAL_CFLAGS := $(COMMON_ALSA_UTILS_CFLAGS)
LOCAL_C_INCLUDES := $(COMMON_ALSA_UTILS_INCLUDES)

include $(BUILD_EXECUTABLE)
