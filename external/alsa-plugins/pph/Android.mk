LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libasound_module_rate_speexrate
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := rate_speexrate.c
LOCAL_SHARED_LIBRARIES := libasound libspeexdsp
LOCAL_REQUIRED_MODULES := libspeexdsp_symlink
LOCAL_CFLAGS := $(COMMON_ALSA_PLUGINS_CFLAGS)
LOCAL_C_INCLUDES := $(COMMON_ALSA_PLUGINS_INCLUDES) $(LOCAL_PATH)/../../speexdsp/prebuilt/system/include
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libasound_module_rate_speexrate_best
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := libasound_module_rate_speexrate
include $(BUILD_PHONY_PACKAGE)

.PHONY: create_speexrate_best_symlink
create_speexrate_best_symlink: libasound_module_rate_speexrate
	mkdir -p $(TARGET_OUT)/usr/lib/alsa-lib
	ln -snf /system/lib/libasound_module_rate_speexrate.so $(TARGET_OUT)/usr/lib/alsa-lib/libasound_module_rate_speexrate_best.so

$(LOCAL_BUILT_MODULE): create_speexrate_best_symlink
