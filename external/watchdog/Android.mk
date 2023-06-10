LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES :=\
	src/command_listener.cpp \
	src/custom.c \
	src/file_stat.c \
	src/file_table.c \
	src/fstab.c \
	src/iface.c \
	src/ifdown.c \
	src/keep_alive.c \
	src/load.c \
	src/lomount.c \
	src/memory.c \
	src/mntent.c \
	src/mount.c \
	src/net.c \
	src/nfsmount.c \
	src/nfsmount_clnt.c \
	src/nfsmount_xdr.c \
	src/pidfile.c \
	src/shutdown.c \
	src/sundries.c \
	src/sync_fts_prop.c \
	src/temp.c \
	src/test_binary.c \
	src/umount.c \
	src/version.c \
	src/watchdog.c \
	src/watchdog_log.c \
	src/watchdog_service.cpp \
	src/watchdog_upload_util.c
LOCAL_CFLAGS := -DHAVE_CONFIG_H -std=gnu99
LOCAL_CPPFLAGS := -DHAVE_CONFIG_H -std=gnu++11
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include bootable/recovery vendor/eureka/flash_ts
LOCAL_SHARED_LIBRARIES := libsysutils libcutils
LOCAL_STATIC_LIBRARIES := $(TARGET_RECOVERY_LIBS)
LOCAL_MODULE := watchdog
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES += wd.dmp init_properties
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/init_properties.c src/sync_fts_prop.c
LOCAL_MODULE := init_properties
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_CFLAGS := -Wall -Werror -std=gnu99
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include vendor/eureka/flash_ts
LOCAL_SHARED_LIBRARIES := libcutils
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := wd.dmp
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/chrome/watchdog
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
