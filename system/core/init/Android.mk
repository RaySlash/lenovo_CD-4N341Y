# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	builtins.c \
	init.c \
	devices.c \
	property_service.c \
	util.c \
	parser.c \
	logo.c \
	keychords.c \
	signal_handler.c \
	init_parser.c \
	ueventd.c \
	ueventd_parser.c

LOCAL_CFLAGS    += -std=c11

ifeq ($(strip $(INIT_BOOTCHART)),true)
LOCAL_SRC_FILES += bootchart.c
LOCAL_CFLAGS    += -DBOOTCHART=1
endif

LOCAL_MODULE:= init
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_UNSTRIPPED)

LOCAL_STATIC_LIBRARIES := libcutils libglibc_bridge

ifeq ($(BUILD_EUREKA),true)
LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX),true)
LOCAL_STATIC_LIBRARIES += libselinux
LOCAL_C_INCLUDES := external/libselinux/include
LOCAL_CFLAGS += -DHAVE_SELINUX
endif

include $(BUILD_EXECUTABLE)

# Make a symlink from /sbin/ueventd to /init
SYMLINKS := $(TARGET_ROOT_OUT)/sbin/ueventd $(TARGET_ROOT_OUT)/sbin/init
$(SYMLINKS): INIT_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> ../$(INIT_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf ../$(INIT_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)
