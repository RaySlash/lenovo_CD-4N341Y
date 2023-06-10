# Google Android makefile for curl and libcurl
#
# This file is an updated version of Dan Fandrich's Android.mk, meant to build
# curl for ToT android with the android build system.

LOCAL_PATH:= $(call my-dir)

# Curl needs a version string.
# As this will be compiled on multiple platforms, generate a version string from
# the build environment variables.
version_string := "Android $(PLATFORM_VERSION) $(TARGET_ARCH_VARIANT)"

curl_CFLAGS := -Wpointer-arith -Wwrite-strings -Wunused -Winline \
	-Wnested-externs -Wmissing-declarations -Wmissing-prototypes -Wno-long-long \
	-Wfloat-equal -Wno-multichar -Wno-sign-compare -Wno-format-nonliteral \
	-Wendif-labels -Wstrict-prototypes -Wdeclaration-after-statement \
	-Wno-system-headers -DHAVE_CONFIG_H -DOS='$(version_string)' -Werror

# Bug: http://b/29823425 Disable -Wvarargs for Clang update to r271374
curl_CFLAGS += -Wno-varargs

curl_includes := \
	$(LOCAL_PATH)/include/ \
	$(LOCAL_PATH)/lib \
	external/zlib

curl_headers := \
        curl.h \
        curlver.h \
        easy.h \
        mprintf.h \
        multi.h \
        stdcheaders.h \
        typecheck-gcc.h

curl_CFLAGS += -DBUILDING_LIBCURL=1
ifeq ($(BUILD_EUREKA),true)
ifeq ($(TARGET_ARCH),arm64)
curl_CFLAGS += -DSIZEOF_LONG=8 \
               -DSIZEOF_LONG_LONG=8 \
               -DSIZEOF_SIZE_T=8 \
               -DSIZEOF_TIME_T=8
else
curl_CFLAGS += -DSIZEOF_LONG=4 \
               -DSIZEOF_LONG_LONG=8 \
               -DSIZEOF_SIZE_T=4 \
               -DSIZEOF_TIME_T=4
endif
endif  # BUILD_EUREKA #

#########################
# Build the libcurl static library

include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib/Makefile.inc

LOCAL_SRC_FILES := $(addprefix lib/,$(CSOURCES))
LOCAL_C_INCLUDES := $(curl_includes)
LOCAL_CFLAGS := $(curl_CFLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE:= libcurl
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libcrypto_static libz

include $(BUILD_STATIC_LIBRARY)

#########################
# Build the libcurl shared library

libcurl_shared_libs := libcrypto libssl libz
libcurl_host_shared_libs := libcrypto-host libssl-host libz-host

include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib/Makefile.inc

LOCAL_SRC_FILES := $(addprefix lib/,$(CSOURCES))
LOCAL_C_INCLUDES := $(curl_includes)
LOCAL_CFLAGS := \
    $(curl_CFLAGS) \
    -D_GNU_SOURCE=1
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE:= libcurl-host
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(libcurl_host_shared_libs)
LOCAL_LDFLAGS := -lpthread

include $(BUILD_HOST_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib/Makefile.inc

LOCAL_SRC_FILES := $(addprefix lib/,$(CSOURCES))
LOCAL_C_INCLUDES := $(curl_includes)
LOCAL_CFLAGS := $(curl_CFLAGS)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE:= libcurl
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := $(libcurl_shared_libs)

LOCAL_COPY_HEADERS_TO := libcurl/curl
LOCAL_COPY_HEADERS := $(addprefix include/curl/,$(curl_headers))

# Copy the headers to the build sysroot directory
include_dir := $(LOCAL_PATH)/include
include_files := $(shell find $(include_dir)/curl -type f -name '*.h')
LOCAL_TOOLCHAIN_PREBUILTS := \
        $(join $(include_files),\
        $(patsubst $(include_dir)/%,:usr/include/%,$(include_files)))

include $(BUILD_SHARED_LIBRARY)

#########################
# Build the curl binary

include $(CLEAR_VARS)
include $(LOCAL_PATH)/src/Makefile.inc
LOCAL_SRC_FILES := $(addprefix src/,$(CURL_CFILES))

LOCAL_MODULE := curl
LOCAL_MODULE_TAGS := eng
LOCAL_STATIC_LIBRARIES := libcurl
LOCAL_SHARED_LIBRARIES := libcrypto libssl libz

LOCAL_C_INCLUDES := $(curl_includes)

# This may also need to include $(CURLX_CFILES) in order to correctly link
# if libcurl is changed to be built as a dynamic library
LOCAL_CFLAGS := $(curl_CFLAGS)

include $(BUILD_EXECUTABLE)

ifeq ($(BUILD_EUREKA),true)
#########################
# Build static curl lib with c-ares and openssl

include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib/Makefile.inc

LOCAL_SRC_FILES := $(addprefix lib/,$(CSOURCES))
LOCAL_C_INCLUDES := $(curl_includes) external/ares external/nghttp2/lib/includes
LOCAL_CFLAGS := $(curl_CFLAGS) \
                -DUSE_ARES=1 \
                -DUSE_NGHTTP2=1 \
                -Wno-unused-parameter

LOCAL_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE:= libcurl_nf_static
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libssl_nf_static libcrypto_nf_static
LOCAL_SHARED_LIBRARIES := libz libcares libnghttp2

include $(BUILD_STATIC_LIBRARY)
endif # BUILD_EUREKA #
