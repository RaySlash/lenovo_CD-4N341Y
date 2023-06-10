LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := i2cget
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := i2cget.c i2cbusses.c util.c ../lib/smbus.c
LOCAL_CFLAGS := -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
                -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
                -W -Wundef -Wmissing-prototypes -Iinclude
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := i2cset
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := i2cset.c i2cbusses.c util.c ../lib/smbus.c
LOCAL_CFLAGS := -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
                -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
                -W -Wundef -Wmissing-prototypes -Iinclude
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := i2cdetect
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := i2cdetect.c i2cbusses.c util.c ../lib/smbus.c
LOCAL_CFLAGS := -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
                -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
                -W -Wundef -Wmissing-prototypes -Iinclude
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := i2cdump
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := i2cdump.c i2cbusses.c util.c ../lib/smbus.c
LOCAL_CFLAGS := -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
                -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
                -W -Wundef -Wmissing-prototypes -Iinclude
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := i2ctransfer
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := i2ctransfer.c i2cbusses.c util.c ../lib/smbus.c
LOCAL_CFLAGS := -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
                -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
                -W -Wundef -Wmissing-prototypes -Iinclude
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
include $(BUILD_EXECUTABLE)
