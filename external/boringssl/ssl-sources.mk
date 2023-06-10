LOCAL_ADDITIONAL_DEPENDENCIES += $(LOCAL_PATH)/eureka.mk
include $(LOCAL_PATH)/eureka.mk

LOCAL_CFLAGS += -I$(LOCAL_PATH)/src/include -Wno-unused-parameter -DBORINGSSL_ANDROID_SYSTEM
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES += $(ssl_sources)
