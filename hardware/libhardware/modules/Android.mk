ifneq ($(BUILD_EUREKA),true)
hardware_modules := \
    audio_remote_submix \
    camera \
    gralloc \
    hwcomposer \
    input \
    radio \
    sensors \
    thermal \
    usbaudio \
    usbcamera \
    vehicle \
    vr
endif  # !BUILD_EUREKA
include $(call all-named-subdir-makefiles,$(hardware_modules))
