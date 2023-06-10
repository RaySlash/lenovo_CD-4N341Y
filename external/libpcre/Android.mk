LOCAL_PATH := $(call my-dir)

pcre2_src_files := \
    dist2/src/pcre2_auto_possess.c \
    dist2/src/pcre2_compile.c \
    dist2/src/pcre2_config.c \
    dist2/src/pcre2_context.c \
    dist2/src/pcre2_convert.c \
    dist2/src/pcre2_dfa_match.c \
    dist2/src/pcre2_error.c \
    dist2/src/pcre2_extuni.c \
    dist2/src/pcre2_find_bracket.c \
    dist2/src/pcre2_maketables.c \
    dist2/src/pcre2_match.c \
    dist2/src/pcre2_match_data.c \
    dist2/src/pcre2_jit_compile.c \
    dist2/src/pcre2_newline.c \
    dist2/src/pcre2_ord2utf.c \
    dist2/src/pcre2_pattern_info.c \
    dist2/src/pcre2_serialize.c \
    dist2/src/pcre2_string_utils.c \
    dist2/src/pcre2_study.c \
    dist2/src/pcre2_substitute.c \
    dist2/src/pcre2_substring.c \
    dist2/src/pcre2_tables.c \
    dist2/src/pcre2_ucd.c \
    dist2/src/pcre2_valid_utf.c \
    dist2/src/pcre2_xclass.c \
    dist2/src/pcre2_chartables.c

include $(CLEAR_VARS)
LOCAL_MODULE := libpcre2
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(pcre2_src_files)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include_internal
LOCAL_CFLAGS := \
    -DHAVE_CONFIG_H \
    -Wall \
    -Werror
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpcre2
LOCAL_MODULE_TAGS := optional
LOCAL_WHOLE_STATIC_LIBRARIES := libpcre2
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpcre2
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(pcre2_src_files)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include_internal
LOCAL_CFLAGS := \
    -DHAVE_CONFIG_H \
    -Wall \
    -Werror
include $(BUILD_HOST_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpcrecpp
LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES := \
    pcrecpp/pcrecpp.cc \
    pcrecpp/pcre_scanner.cc \
    pcrecpp/pcre_stringpiece.cc

LOCAL_SHARED_LIBRARIES := libpcre2

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/pcrecpp/include

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/pcrecpp/include

LOCAL_CFLAGS := \
    -Wall \
    -Werror \
    -Wno-unused-parameter \
    -Wno-unused-variable
include $(BUILD_SHARED_LIBRARY)
