# Now used only by Trusty
LOCAL_ADDITIONAL_DEPENDENCIES += $(LOCAL_PATH)/eureka.mk
include $(LOCAL_PATH)/eureka.mk

LOCAL_CFLAGS += -I$(LOCAL_PATH)/src/include -I$(LOCAL_PATH)/src/crypto -Wno-unused-parameter -DBORINGSSL_ANDROID_SYSTEM
LOCAL_ASFLAGS += -I$(LOCAL_PATH)/src/include -I$(LOCAL_PATH)/src/crypto -Wno-unused-parameter
# Do not add in the architecture-specific files if we don't want to build assembly
LOCAL_SRC_FILES_x86 := $(linux_x86_sources)
LOCAL_SRC_FILES_x86_64 := $(linux_x86_64_sources)
LOCAL_SRC_FILES_arm := $(linux_arm_sources)
LOCAL_SRC_FILES_arm64 := $(linux_aarch64_sources)
LOCAL_SRC_FILES += $(crypto_sources)

# TODO(tiansong): Our build system doesn't support LOCAL_SRC_FILES_arm
ifeq ($(BUILD_EUREKA),true)
ifeq ($(LOCAL_IS_HOST_MODULE), true)

ifeq (x86_64,$(findstring x86_64,$(UNAME)))
LOCAL_SRC_FILES += $(linux_x86_64_sources)
else
LOCAL_SRC_FILES += $(linux_x86_sources)
endif  # Linux x86_64

else ifeq ($(TARGET_ARCH),arm)
LOCAL_SRC_FILES += $(linux_arm_sources)
else ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES += $(linux_aarch64_sources)
else ifeq ($(TARGET_ARCH),x86-64)
LOCAL_SRC_FILES += $(linux_x86_64_sources)
else ifeq ($(TARGET_ARCH),mipsel)
# No assembly files; fall through.
else
# If this condition is met, consider adding cpu-specific files to
# LOCAL_SRC_FILES. If there are no such files, add a fall-through condition.
$(error No asm files found for $(TARGET_ARCH)!)
endif  # LOCAL_IS_HOST_MODULE == true
endif  # BUILD_EUREKA == true
