LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := aplay
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := aplay.c
LOCAL_SHARED_LIBRARIES := libasound
LOCAL_CFLAGS := $(COMMON_ALSA_UTILS_CFLAGS)
LOCAL_C_INCLUDES := $(COMMON_ALSA_UTILS_INCLUDES)
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := arecord
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := aplay
include $(BUILD_PHONY_PACKAGE)

.PHONY: create_arecord_symlink
create_arecord_symlink: aplay
	cd $(TARGET_OUT_EXECUTABLES) && ln -sf aplay arecord

$(LOCAL_BUILT_MODULE): create_arecord_symlink
