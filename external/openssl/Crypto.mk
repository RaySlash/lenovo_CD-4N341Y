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
#LOCAL_C_INCLUDES := $(log_c_includes) \
 #                   $(LOCAL_PATH)/crypto \
  #                  $(LOCAL_PATH)/include \
   #                 $(LOCAL_PATH)/crypto/asn1 \
    #                $(LOCAL_PATH)/crypto/evp \
     #               $(LOCAL_PATH)/crypto/modes
# The static library should be used in only unbundled apps
# and we don't have clang in unbundled build yet.
LOCAL_SDK_VERSION := 9

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcrypto_nf_static
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Crypto.mk
 
LOCAL_SRC_FILES_arm :=
LOCAL_CFLAGS_arm :=
LOCAL_SRC_FILES_arm64 :=
LOCAL_CFLAGS_arm64 :=
LOCAL_SRC_FILES_x86_64 :=
LOCAL_CFLAGS_x86_64 :=
include $(LOCAL_PATH)/Crypto-config-target.mk
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

# Replace cflags with static-specific cflags so we dont build in libdl deps
LOCAL_CFLAGS_32 := $(openssl_cflags_static_32)
LOCAL_CFLAGS_64 := $(openssl_cflags_static_64)
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
LOCAL_LDFLAGS += -ldl

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcrypto_nf
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Crypto.mk

# Chromecast's make system doesn't support proper multi-arch yet.
# Manually reset *_arm and set LOCAL_CFLAGS and LOCAL_SRC_FILES.
LOCAL_SRC_FILES_arm :=
LOCAL_CFLAGS_arm :=
LOCAL_SRC_FILES_arm64 :=
LOCAL_CFLAGS_arm64 :=
LOCAL_SRC_FILES_x86_64 :=
LOCAL_CFLAGS_x86_64 :=
include $(LOCAL_PATH)/Crypto-config-target.mk
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
LOCAL_CFLAGS += -DPURIFY
LOCAL_LDLIBS += -ldl
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcrypto_nf-host
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Crypto.mk
LOCAL_MULTILIB := both
include $(LOCAL_PATH)/Crypto-config-host.mk
include $(LOCAL_PATH)/android-config.mk
include $(BUILD_HOST_SHARED_LIBRARY)

########################################
# host static library, which is used by some SDK tools.

include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := $(log_shared_libraries)
LOCAL_C_INCLUDES := $(log_c_includes) \
                    $(LOCAL_PATH)/crypto \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/crypto/asn1 \
                    $(LOCAL_PATH)/crypto/evp \
                    $(LOCAL_PATH)/crypto/modes
#LOCAL_C_INCLUDES := $(log_c_includes)
LOCAL_CFLAGS += -DPURIFY
LOCAL_LDLIBS += -ldl
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcrypto_nf_static
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Crypto.mk
include $(LOCAL_PATH)/Crypto-config-host.mk
include $(LOCAL_PATH)/android-config.mk
include $(BUILD_HOST_STATIC_LIBRARY)
endif # BUILD_EUREKA
