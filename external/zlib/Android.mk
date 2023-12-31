LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# measurements show that the ARM version of ZLib is about x1.17 faster
# than the thumb one...
LOCAL_ARM_MODE := arm

zlib_files := \
	src/adler32.c \
	src/compress.c \
	src/crc32.c \
	src/deflate.c \
	src/gzclose.c \
	src/gzlib.c \
	src/gzread.c \
	src/gzwrite.c \
	src/infback.c \
	src/inflate.c \
	src/inftrees.c \
	src/inffast.c \
	src/trees.c \
	src/uncompr.c \
	src/zutil.c

LOCAL_MODULE := libz
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -O3 -DUSE_MMAP

# TODO: This is to work around b/24465209. Remove after root cause is fixed
LOCAL_LDFLAGS_arm := -Wl,--hash-style=both

LOCAL_SRC_FILES := $(zlib_files)
LOCAL_CXX_STL := none
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_TOOLCHAIN_PREBUILTS := \
        $(LOCAL_PATH)/src/zlib.h:usr/include/zlib.h \
        $(LOCAL_PATH)/src/zconf.h:usr/include/zconf.h
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libz
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -O3 -DUSE_MMAP
LOCAL_SRC_FILES := $(zlib_files)
LOCAL_CXX_STL := none
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_TOOLCHAIN_PREBUILTS := \
        $(LOCAL_PATH)/src/zlib.h:usr/include/zlib.h \
        $(LOCAL_PATH)/src/zconf.h:usr/include/zconf.h
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libz
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -O3 -DUSE_MMAP
LOCAL_SRC_FILES := $(zlib_files)
LOCAL_MULTILIB := both
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_MODULE_HOST_OS := darwin linux windows
LOCAL_CXX_STL := none
include $(BUILD_HOST_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libz-host
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -O3 -DUSE_MMAP
LOCAL_SRC_FILES := $(zlib_files)
LOCAL_MULTILIB := both
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_CXX_STL := none
include $(BUILD_HOST_SHARED_LIBRARY)

unzip_files := \
        src/adler32.c \
        src/crc32.c \
        src/inflate.c \
        src/inftrees.c \
        src/inffast.c \
        src/zutil.c

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(unzip_files)
LOCAL_MODULE := libunz
LOCAL_ARM_MODE := arm
include $(BUILD_HOST_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(unzip_files)
LOCAL_MODULE := libunz
LOCAL_ARM_MODE := arm
LOCAL_TOOLCHAIN_PREBUILTS := \
        $(LOCAL_PATH)/src/zlib.h:usr/include/zlib.h \
        $(LOCAL_PATH)/src/zconf.h:usr/include/zconf.h
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=        \
	src/test/minigzip.c

LOCAL_MODULE:= gzip

LOCAL_SHARED_LIBRARIES := libz

LOCAL_CXX_STL := none

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=        \
	src/test/minigzip.c

LOCAL_MODULE:= minigzip

LOCAL_STATIC_LIBRARIES := libz

LOCAL_CXX_STL := none

include $(BUILD_HOST_EXECUTABLE)

$(TARGET_OUT_COMMON_GEN)/zlib_fingerprint : $(wildcard $(LOCAL_PATH)/src/*.[ch])
	printf '%s\n' $^ | LC_ALL=C sort | xargs cat | shasum -a 256 | \
		awk '{printf $$1}' > $@
