# Note that some host libraries have the same module name as the target
# libraries. This is currently needed to build, for example, adb. But it's
# probably something that should be changed.

LOCAL_PATH := $(call my-dir)

boringssl_cflags := \
    -fvisibility=hidden \
    -DBORINGSSL_SHARED_LIBRARY \
    -DBORINGSSL_IMPLEMENTATION \
    -DOPENSSL_SMALL \
    -D_XOPEN_SOURCE=700 \
    -Wno-unused-parameter \
    -DBORINGSSL_ANDROID_SYSTEM

boringssl_cppflags := \
    -Wall \
    -Werror

boringssl_conlyflags := \
    -std=c99

## libcrypto

# Target static library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcrypto_static
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/crypto-sources.mk
LOCAL_SDK_VERSION := 9
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
# sha256-armv4.S does not compile with clang.
LOCAL_CLANG_ASFLAGS_arm += -no-integrated-as
LOCAL_CLANG_ASFLAGS_arm64 += -march=armv8-a+crypto
include $(LOCAL_PATH)/crypto-sources.mk
include $(BUILD_STATIC_LIBRARY)

# Target shared library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcrypto
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/crypto-sources.mk
LOCAL_SDK_VERSION := 9
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
# sha256-armv4.S does not compile with clang.
LOCAL_CLANG_ASFLAGS_arm += -no-integrated-as
LOCAL_CLANG_ASFLAGS_arm64 += -march=armv8-a+crypto

ifneq ($(BUILD_EUREKA),)
# Copy the headers to the build sysroot directory.
include_dir := $(LOCAL_PATH)/include
include_files := $(shell find $(include_dir)/openssl -type f -name '*.h')
LOCAL_TOOLCHAIN_PREBUILTS := \
	$(join $(include_files),\
	$(patsubst $(include_dir)/%,:usr/include/%,$(include_files)))
LOCAL_ADDITIONAL_DEPENDENCIES := $(local_additional_dependencies)
endif # BUILD_EUREKA

include $(LOCAL_PATH)/crypto-sources.mk
include $(BUILD_SHARED_LIBRARY)

# Target static tool
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := cc
LOCAL_MODULE := bssl
LOCAL_MODULE_TAGS := eng
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/sources.mk
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
LOCAL_SHARED_LIBRARIES=libcrypto libssl
include $(LOCAL_PATH)/sources.mk
LOCAL_SRC_FILES = $(tool_sources)
include $(BUILD_EXECUTABLE)

ifneq ($(BUILD_EUREKA),true)
# Host static library
include $(CLEAR_VARS)
LOCAL_IS_HOST_MODULE := true
LOCAL_MODULE := libcrypto_static
LOCAL_MODULE_HOST_OS := darwin linux windows
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/crypto-sources.mk
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
LOCAL_CXX_STL := none
# Windows and Macs both have problems with assembly files
ifeq ($(HOST_OS),darwin)
LOCAL_CFLAGS += -DOPENSSL_NO_ASM
else ifeq ($(HOST_OS),windows)
LOCAL_CFLAGS += -DOPENSSL_NO_ASM
endif  # HOST_OS
# TODO: b/26097626. ASAN breaks use of this library in JVM.
# Re-enable sanitization when the issue with making clients of this library
# preload ASAN runtime is resolved. Without that, clients are getting runtime
# errors due to unresoled ASAN symbols, such as
# __asan_option_detect_stack_use_after_return.
LOCAL_SANITIZE := never
include $(LOCAL_PATH)/crypto-sources.mk
include $(BUILD_HOST_STATIC_LIBRARY)
endif  # $(BUILD_EUREKA) != true

# Host shared library
include $(CLEAR_VARS)
LOCAL_IS_HOST_MODULE := true
LOCAL_MODULE := libcrypto-host
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/include $(LOCAL_PATH)/src/crypto
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_MULTILIB := both
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/crypto-sources.mk
LOCAL_CFLAGS := $(boringssl_cflags) -DBORINGSSL_IMPLEMENTATION
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
# Windows and Macs both have problems with assembly files
ifeq ($(HOST_OS),linux)
LOCAL_LDLIBS := -lpthread
else ifeq ($(HOST_OS),darwin)
LOCAL_CFLAGS += -DOPENSSL_NO_ASM
LOCAL_LDLIBS := -lpthread
else ifeq ($(HOST_OS),windows)
LOCAL_CFLAGS += -DOPENSSL_NO_ASM
LOCAL_LDFLAGS += -lws2_32
else
$(error HOST_OS must be one of linux, darwin, or windows!)
endif  # HOST_OS

LOCAL_CXX_STL := none
include $(LOCAL_PATH)/crypto-sources.mk
include $(BUILD_HOST_SHARED_LIBRARY)

## libssl

# Target static library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libssl_static
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/ssl-sources.mk
LOCAL_SDK_VERSION := 9
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
include $(LOCAL_PATH)/ssl-sources.mk
include $(BUILD_STATIC_LIBRARY)

# Target shared library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libssl
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/ssl-sources.mk
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
LOCAL_SHARED_LIBRARIES=libcrypto
LOCAL_SDK_VERSION := 9
include $(LOCAL_PATH)/ssl-sources.mk
include $(BUILD_SHARED_LIBRARY)

ifneq ($(BUILD_EUREKA),true)
# Host static library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libssl_static-host
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/ssl-sources.mk
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
LOCAL_CXX_STL := none
# TODO: b/26097626. ASAN breaks use of this library in JVM.
# Re-enable sanitization when the issue with making clients of this library
# preload ASAN runtime is resolved. Without that, clients are getting runtime
# errors due to unresoled ASAN symbols, such as
# __asan_option_detect_stack_use_after_return.
LOCAL_SANITIZE := never
include $(LOCAL_PATH)/ssl-sources.mk
include $(BUILD_HOST_STATIC_LIBRARY)
endif  # $(BUILD_EUREKA) != true

ifneq ($(BUILD_EUREKA),true)
# Host static tool (for linux only).
ifeq ($(HOST_OS), linux)
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := cc
LOCAL_MODULE := bssl
LOCAL_MODULE_TAGS := optional
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/sources.mk
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
LOCAL_SHARED_LIBRARIES=libcrypto-host libssl-host
# Needed for clock_gettime.
LOCAL_LDFLAGS := -lrt
include $(LOCAL_PATH)/sources.mk
LOCAL_SRC_FILES = $(tool_sources)
include $(BUILD_HOST_EXECUTABLE)
endif  # HOST_OS == linux
endif  # $(BUILD_EUREKA) != true

# Host shared library
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libssl-host
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/src/include
LOCAL_MULTILIB := both
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk $(LOCAL_PATH)/ssl-sources.mk
LOCAL_CFLAGS := $(boringssl_cflags)
LOCAL_CPPFLAGS := $(boringssl_cppflags)
LOCAL_CONLYFLAGS := $(boringssl_conlyflags)
LOCAL_CXX_STL := none
LOCAL_SHARED_LIBRARIES += libcrypto-host
include $(LOCAL_PATH)/ssl-sources.mk
include $(BUILD_HOST_SHARED_LIBRARY)
