LOCAL_PATH := $(call my-dir)

liblog_files := \
    config_read.c \
    config_write.c \
    local_logger.c \
    log_event_list.c \
    log_event_write.c \
    log_ratelimit.cpp \
    logger_lock.c \
    logger_name.c \
    logger_read.c \
    logger_write.c \
    logprint.c \
    stderr_write.c

liblog_host_files := \
    fake_log_device.c \
    fake_writer.c

liblog_target_files := \
    event_tag_map.cpp \
    log_time.cpp \
    properties.c \
    pmsg_reader.c \
    pmsg_writer.c \
    logd_reader.c \
    logd_writer.c

cflags := -D_XOPEN_SOURCE \
    -D_GNU_SOURCE \
    -Werror \
    -Wno-unused-parameter \
    -DLIBLOG_LOG_TAG=1006 \
    -fvisibility=hidden \
    -DSNET_EVENT_LOG_TAG=1397638484

liblog_export_headers := \
    android/log.h \
    log/log.h \
    log/log_id.h \
    log/log_main.h \
    log/log_radio.h \
    log/log_to_buffer.h \
    log/log_read.h \
    log/log_safetynet.h \
    log/log_system.h \
    log/log_time.h \
    log/uio.h

include $(CLEAR_VARS)
LOCAL_MODULE := liblog
LOCAL_LDLIBS += -lpthread
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(liblog_files) $(liblog_host_files)
LOCAL_CFLAGS := $(cflags) -DFAKE_LOG_DEVICE=1

LOCAL_TOOLCHAIN_PREBUILTS := $(join \
    $(addprefix $(LOCAL_PATH)/include/,$(liblog_export_headers)),\
    $(addprefix :usr/include/,$(liblog_export_headers)))

include $(BUILD_HOST_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := liblog
LOCAL_LDLIBS += -lpthread
LOCAL_CFLAGS :=  $(cflags)
LOCAL_WHOLE_STATIC_LIBRARIES := liblog

LOCAL_TOOLCHAIN_PREBUILTS := $(join \
    $(addprefix $(LOCAL_PATH)/include/,$(liblog_export_headers)),\
    $(addprefix :usr/include/,$(liblog_export_headers)))

include $(BUILD_HOST_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= liblog
LOCAL_LDLIBS += -lpthread
LOCAL_SRC_FILES := $(liblog_files) $(liblog_target_files)
LOCAL_CFLAGS := $(cflags)

LOCAL_STATIC_LIBRARIES := libglibc_bridge
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include \
    $(KERNEL_HEADERS)

LOCAL_TOOLCHAIN_PREBUILTS := $(join \
    $(addprefix $(LOCAL_PATH)/include/,$(liblog_export_headers)),\
    $(addprefix :usr/include/,$(liblog_export_headers)))

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := liblog
LOCAL_LDLIBS += -lpthread
LOCAL_CFLAGS := $(cflags)
LOCAL_WHOLE_STATIC_LIBRARIES := liblog
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_LIB)

LOCAL_TOOLCHAIN_PREBUILTS := $(join \
    $(addprefix $(LOCAL_PATH)/include/,$(liblog_export_headers)),\
    $(addprefix :usr/include/,$(liblog_export_headers)))

include $(BUILD_SHARED_LIBRARY)

include $(call all-subdir-makefiles)
