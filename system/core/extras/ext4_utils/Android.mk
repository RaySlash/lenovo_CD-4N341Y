# Copyright 2010 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)

libext4_utils_src_files := \
    make_ext4fs.c \
    ext4fixup.c \
    ext4_utils.c \
    allocate.c \
    contents.c \
    extent.c \
    indirect.c \
    uuid.c \
    sha1.c \
    wipe.c

#
# -- All host/targets including windows
#

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(libext4_utils_src_files)
LOCAL_MODULE := libext4_utils_host
LOCAL_C_INCLUDES += external/zlib
LOCAL_STATIC_LIBRARIES += libsparse_host
ifeq ($(BUILD_EUREKA),true)
  LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX), true)
  LOCAL_C_INCLUDES += external/libselinux/include
  LOCAL_STATIC_LIBRARIES += libselinux
  LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
include $(BUILD_HOST_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := make_ext4fs_main.c
LOCAL_MODULE := make_ext4fs
LOCAL_STATIC_LIBRARIES += \
    libext4_utils_host \
    libsparse_host \
    libfsconfig \
    libz
ifeq ($(HOST_OS),windows)
  LOCAL_LDLIBS += -lws2_32
else
  ifeq ($(BUILD_EUREKA),true)
    LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
  endif # BUILD_EUREKA
  ifeq ($(HAVE_SELINUX), true)
    LOCAL_C_INCLUDES += external/libselinux/include
    LOCAL_STATIC_LIBRARIES += libselinux
    LOCAL_CFLAGS += -DHAVE_SELINUX
  endif # HAVE_SELINUX
endif
include $(BUILD_HOST_EXECUTABLE)
$(call dist-for-goals, dist_files, $(LOCAL_BUILT_MODULE))


#
# -- All host/targets excluding windows
#

ifneq ($(HOST_OS),windows)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(libext4_utils_src_files)
LOCAL_MODULE := libext4_utils
LOCAL_C_INCLUDES += external/zlib
LOCAL_SHARED_LIBRARIES := \
    libsparse \
    libz
ifeq ($(BUILD_EUREKA),true)
  LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX), true)
  LOCAL_C_INCLUDES += external/libselinux/include
  LOCAL_SHARED_LIBRARIES += libselinux
  LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(libext4_utils_src_files)
LOCAL_MODULE := libext4_utils_static
LOCAL_C_INCLUDES += external/zlib
LOCAL_STATIC_LIBRARIES += \
    libsparse_static
ifeq ($(BUILD_EUREKA),true)
  LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX), true)
  LOCAL_C_INCLUDES += external/libselinux/include
  LOCAL_STATIC_LIBRARIES += libselinux
  LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := make_ext4fs_main.c
LOCAL_MODULE := make_ext4fs
LOCAL_STATIC_LIBRARIES := libfsconfig
LOCAL_SHARED_LIBRARIES += libext4_utils libz
ifeq ($(BUILD_EUREKA),true)
  LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX), true)
  LOCAL_C_INCLUDES += external/libselinux/include
  LOCAL_SHARED_LIBRARIES += libselinux
  LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := ext2simg.c
LOCAL_MODULE := ext2simg
LOCAL_SHARED_LIBRARIES += \
    libext4_utils \
    libsparse \
    libz
ifeq ($(BUILD_EUREKA),true)
  LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX), true)
  LOCAL_C_INCLUDES += external/libselinux/include
  LOCAL_SHARED_LIBRARIES += libselinux
  LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := ext2simg.c
LOCAL_MODULE := ext2simg
LOCAL_STATIC_LIBRARIES += \
    libext4_utils_host \
    libsparse_host \
    libz
ifeq ($(BUILD_EUREKA),true)
  LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX), true)
  LOCAL_C_INCLUDES += external/libselinux/include
  LOCAL_STATIC_LIBRARIES += libselinux
  LOCAL_CFLAGS += -DHAVE_SELINUX
endif # HAVE_SELINUX
include $(BUILD_HOST_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := setup_fs.c
LOCAL_MODULE := setup_fs
LOCAL_SHARED_LIBRARIES += libcutils
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := ext4fixup_main.c
LOCAL_MODULE := ext4fixup
LOCAL_SHARED_LIBRARIES += \
    libext4_utils \
    libsparse \
    libz
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := ext4fixup_main.c
LOCAL_MODULE := ext4fixup
LOCAL_STATIC_LIBRARIES += \
    libext4_utils_host \
    libsparse_host \
    libz
include $(BUILD_HOST_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_MODULE := mkuserimg.sh
LOCAL_SRC_FILES := mkuserimg.sh
LOCAL_MODULE_CLASS := EXECUTABLES
# We don't need any additional suffix.
LOCAL_MODULE_SUFFIX :=
LOCAL_BUILT_MODULE_STEM := $(notdir $(LOCAL_SRC_FILES))
LOCAL_IS_HOST_MODULE := true
include $(BUILD_PREBUILT)

endif
