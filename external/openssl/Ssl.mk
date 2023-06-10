#######################################
# target static library
include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := $(log_shared_libraries)

LOCAL_C_INCLUDES := $(log_c_includes) \
                    $(LOCAL_PATH)/crypto \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/crypto/asn1 \
                    $(LOCAL_PATH)/crypto/evp \
                    $(LOCAL_PATH)/crypto/modes
# The static library should be used in only unbundled apps
# and we don't have clang in unbundled build yet.
LOCAL_SDK_VERSION := 9

LOCAL_SRC_FILES += $(target_src_files)
LOCAL_CFLAGS += $(target_c_flags)
LOCAL_C_INCLUDES += $(target_c_includes)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libssl_nf_static
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Ssl.mk
LOCAL_SRC_FILES_arm :=
LOCAL_CFLAGS_arm :=
LOCAL_SRC_FILES_arm64 :=
LOCAL_CFLAGS_arm64 :=
LOCAL_SRC_FILES_x86_64 :=
LOCAL_CFLAGS_x86_64 :=
include $(LOCAL_PATH)/Ssl-config-target.mk
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

include $(BUILD_STATIC_LIBRARY)

#######################################
# target shared library
include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := $(log_shared_libraries)
LOCAL_C_INCLUDES := $(log_c_includes) \
                    $(LOCAL_PATH)/crypto \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/crypto/asn1 \
                    $(LOCAL_PATH)/crypto/evp \
                    $(LOCAL_PATH)/crypto/modes
#LOCAL_C_INCLUDES := $(log_c_includes)

# If we're building an unbundled build, don't try to use clang since it's not
# in the NDK yet. This can be removed when a clang version that is fast enough
# in the NDK.
ifeq (,$(TARGET_BUILD_APPS))
LOCAL_CLANG := true
else
LOCAL_SDK_VERSION := 9
endif

LOCAL_SHARED_LIBRARIES += libcrypto_nf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libssl_nf

ifneq ($(BUILD_EUREKA),)
# Copy the headers to the build sysroot directory.
include_dir := $(LOCAL_PATH)/include
include_files := $(shell find $(include_dir)/openssl -type f -name '*.h')
LOCAL_TOOLCHAIN_PREBUILTS := \
	$(join $(include_files),\
	$(patsubst $(include_dir)/%,:usr/include/%,$(include_files)))
LOCAL_ADDITIONAL_DEPENDENCIES := $(local_additional_dependencies)
endif # BUILD_EUREKA

# Chromecast's make system doesn't support proper multi-arch yet.
# Manually reset *_arm and set LOCAL_CFLAGS and LOCAL_SRC_FILES.
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Ssl.mk
LOCAL_SRC_FILES_arm :=
LOCAL_CFLAGS_arm :=
LOCAL_SRC_FILES_arm64 :=
LOCAL_CFLAGS_arm64 :=
LOCAL_SRC_FILES_x86_64 :=
LOCAL_CFLAGS_x86_64 :=
include $(LOCAL_PATH)/Ssl-config-target.mk
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

include $(BUILD_SHARED_LIBRARY)

ifeq ($(BUILD_EUREKA),)
#######################################
# host shared library
include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := $(log_shared_libraries)
LOCAL_C_INCLUDES := $(log_c_includes) \
                    $(LOCAL_PATH)/crypto \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/crypto/asn1 \
                    $(LOCAL_PATH)/crypto/evp \
                    $(LOCAL_PATH)/crypto/modes
#LOCAL_C_INCLUDES := $(log_c_includes)

LOCAL_SHARED_LIBRARIES += libcrypto_nf-host
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libssl_nf-host
LOCAL_MULTILIB := both
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Ssl.mk
include $(LOCAL_PATH)/Ssl-config-host.mk
include $(LOCAL_PATH)/android-config.mk
include $(BUILD_HOST_SHARED_LIBRARY)

#######################################
# ssltest
include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := $(log_shared_libraries)
LOCAL_C_INCLUDES := $(log_c_includes) \
                    $(LOCAL_PATH)/crypto \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/crypto/asn1 \
                    $(LOCAL_PATH)/crypto/evp \
                    $(LOCAL_PATH)/crypto/modes



LOCAL_SRC_FILES := ssl/ssltest.c
LOCAL_SHARED_LIBRARIES := libssl_nf libcrypto_nf
LOCAL_MODULE := ssltest_nf
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := ssltest_nf
LOCAL_MODULE_STEM_64 := ssltest_nf64
LOCAL_MODULE_TAGS := optional
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Ssl.mk
include $(LOCAL_PATH)/Ssl-config-host.mk
include $(LOCAL_PATH)/android-config.mk
include $(BUILD_EXECUTABLE)
endif # BUILD_EUREKA
