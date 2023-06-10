# Copyright 2006 The Android Open Source Project

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := openssl
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := openssl
LOCAL_MODULE_STEM_64 := openssl64
LOCAL_CLANG := true
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libssl_nf_static libcrypto_nf_static

# Chromecast's make system doesn't support proper multi-arch yet.
# Manually reset *_arm and set LOCAL_CFLAGS and LOCAL_SRC_FILES.
LOCAL_SRC_FILES_arm :=
LOCAL_CFLAGS_arm :=
LOCAL_SRC_FILES_arm64 :=
LOCAL_CFLAGS_arm64 :=
LOCAL_SRC_FILES_x86_64 :=
LOCAL_CFLAGS_x86_64 :=
include $(LOCAL_PATH)/Apps-config-target.mk
include $(LOCAL_PATH)/android-config.mk
ifeq ($(TARGET_ARCH),arm64)
LOCAL_CFLAGS += $(openssl_cflags_64) ${LOCAL_CFLAGS_arm64}
LOCAL_SRC_FILES += ${LOCAL_SRC_FILES_arm64}
else
ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += $(openssl_cflags) ${LOCAL_CFLAGS_arm}
LOCAL_SRC_FILES += ${LOCAL_SRC_FILES_arm}
else
ifeq ($(TARGET_ARCH),x86-64)
LOCAL_CFLAGS += $(openssl_cflags_64) ${LOCAL_CFLAGS_x86_64}
LOCAL_SRC_FILES += ${LOCAL_SRC_FILES_x86_64}
endif #x86_64
endif #arm
endif #arm64

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Apps.mk
include $(BUILD_EXECUTABLE)

ifeq ($(BUILD_EUREKA),)
include $(CLEAR_VARS)
LOCAL_MODULE := openssl
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libssl_nf-host libcrypto_nf-host
include $(LOCAL_PATH)/Apps-config-host.mk
include $(LOCAL_PATH)/android-config.mk
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Apps.mk
include $(BUILD_HOST_EXECUTABLE)
endif # BUILD_EUREKA
