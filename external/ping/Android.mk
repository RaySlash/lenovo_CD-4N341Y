LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= ping.c
LOCAL_MODULE := ping
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils
include $(BUILD_EXECUTABLE)
