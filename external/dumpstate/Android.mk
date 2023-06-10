LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := dumpstate.c utils.c

LOCAL_MODULE := dumpstate
LOCAL_MODULE_PATH := $(TARGET_OUT)/chrome
LOCAL_REQUIRED_MODULES := dumpstate_symlink
LOCAL_SHARED_LIBRARIES := libcutils liblog

ifeq ($(TARGET_PRODUCT),$(filter $(TARGET_PRODUCT),valens vento))
LOCAL_SHARED_LIBRARIES += libasound
endif

ifeq ($(TARGET_PRODUCT),$(filter $(TARGET_PRODUCT), elaine newman valens vento))
# TODO(b/139484290): Remove when vendor driver is fixed.
# More details in utils.c
LOCAL_CFLAGS += -DBOARD_DUMPSTATE_FILTER_MAP_DEVZERO
endif

ifdef BOARD_LIB_DUMPSTATE
LOCAL_STATIC_LIBRARIES := $(BOARD_LIB_DUMPSTATE)
LOCAL_CFLAGS += -DBOARD_HAS_DUMPSTATE
endif

LOCAL_CFLAGS += -Werror -std=gnu99

# dumpstate is suid, so we must explicitly include
# any library path dependencies.
LOCAL_LDFLAGS += -Wl,-rpath=/system/lib

include $(BUILD_EXECUTABLE)

# Create symlinks /bin/dumpstate -> /chrome/dumpstate to
# make sure nothing (legacy usage, for example test scripts) breaks.
include $(CLEAR_VARS)
LOCAL_MODULE := dumpstate_symlink
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := acp
include $(BUILD_PHONY_PACKAGE)

.PHONY: create_dumpstate_symlink
create_dumpstate_symlink: dumpstate
	mkdir -p $(TARGET_OUT)/bin
	ln -snf /chrome/dumpstate $(TARGET_OUT)/bin/dumpstate

$(LOCAL_BUILT_MODULE): create_dumpstate_symlink
