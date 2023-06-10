LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

TOOLS := \
	ls \
	mount \
	cat \
	ps \
	kill \
	ln \
	insmod \
	rmmod \
	lsmod \
	setconsole \
	mkdir \
	rmdir \
	reboot \
	sendevent \
	date \
	wipe \
	sync \
	umount \
	start \
	stop \
	notify \
	cmp \
	route \
	hd \
	dd \
	getprop \
	setprop \
	watchprops \
	log \
	sleep \
	renice \
	printenv \
	smd \
	chmod \
	chown \
	newfs_msdos \
	netstat \
	ioctl \
	mv \
	schedtop \
	top \
	iftop \
	id \
	uptime \
	vmstat \
	nandread \
	ionice \
	touch \
	lsof \
	mknod \
	sntpd \
	exists \
	sudo \
	devmem \
	getevent

ifeq ($(HAVE_SELINUX),true)

TOOLS += \
	getenforce \
	setenforce \
	chcon \
	restorecon \
	runcon \
	getsebool \
	setsebool \
	load_policy

endif


ifneq (,$(filter userdebug eng,$(TARGET_BUILD_VARIANT)))
TOOLS += r
endif

LOCAL_SRC_FILES:= \
	dynarray.c \
	toolbox.c \
	$(patsubst %,%.c,$(TOOLS))

ifeq ($(TARGET_USE_SYSTEM_LIBS),true)
LOCAL_CFLAGS += -Dlint
endif

LOCAL_SHARED_LIBRARIES := libcutils

ifeq ($(BUILD_EUREKA),true)
LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
ifeq ($(HAVE_SELINUX),true)
LOCAL_CFLAGS += -DHAVE_SELINUX
LOCAL_SHARED_LIBRARIES += libselinux
LOCAL_C_INCLUDES += external/libselinux/include
endif

LOCAL_MODULE:= toolbox

# Including this will define $(intermediates).
#
include $(BUILD_EXECUTABLE)

$(LOCAL_PATH)/toolbox.c: $(intermediates)/tools.h
$(intermediates)/tools.h:  $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h

TOOLS_H := $(intermediates)/tools.h
$(TOOLS_H): PRIVATE_TOOLS := $(TOOLS)
$(TOOLS_H): PRIVATE_CUSTOM_TOOL = echo "/* file generated automatically */" > $@ ; for t in $(PRIVATE_TOOLS) ; do echo "TOOL($$t)" >> $@ ; done
$(TOOLS_H): $(LOCAL_PATH)/Android.mk
$(TOOLS_H):
	$(transform-generated-source)

# Make #!/system/bin/toolbox launchers for each tool.
#
SYMLINKS := $(addprefix $(TARGET_OUT)/bin/,$(TOOLS))
$(SYMLINKS): TOOLBOX_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> $(TOOLBOX_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(TOOLBOX_BINARY) $@

# Create one module that will wrap all the symlinks.  This module requires the
# toolbox module, then adds an explicit dependency on all symlinks.
include $(CLEAR_VARS)
ifeq ($(BUILD_EUREKA),true)
LOCAL_CFLAGS += -include $(HOST_OUT_COMMON_INTERMEDIATES)/have_selinux.h
endif # BUILD_EUREKA
LOCAL_MODULE := toolbox_symlinks
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := toolbox
include $(BUILD_PHONY_PACKAGE)
$(LOCAL_BUILT_MODULE): $(SYMLINKS)

ALL_DEFAULT_INSTALLED_MODULES += toolbox_symlinks

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) toolbox_symlinks

include $(CLEAR_VARS)
LOCAL_SRC_FILES := sntpd_unittest.c
LOCAL_CFLAGS += -UNDEBUG
LOCAL_MODULE := sntpd_unittest
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

