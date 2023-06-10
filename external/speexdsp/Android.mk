LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libspeexdsp
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := prebuilt/system/lib/libspeexdsp.so
LOCAL_EXPORT_C_INCLUDES := prebuilt/system/include
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libspeexdsp_symlink
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := libspeexdsp
include $(BUILD_PHONY_PACKAGE)

.PHONY: create_libspeexdsp_symlink
create_libspeexdsp_symlink: libspeexdsp
	ln -snf libspeexdsp.so $(TARGET_OUT_SHARED_LIBRARIES)/libspeexdsp.so.1

$(LOCAL_BUILT_MODULE): create_libspeexdsp_symlink
