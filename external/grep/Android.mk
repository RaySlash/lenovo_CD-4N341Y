LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

PSEUDONYMS := egrep zgrep zegrep

LOCAL_SRC_FILES:=                \
                  file.c         \
                  binary.c       \
                  grep.c         \
                  mmfile.c       \
                  queue.c        \
                  util.c

LOCAL_MODULE:= grep
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := external/zlib

# Localize as many of the porting changes as possible to the Makefile
LOCAL_CFLAGS += -DREG_BASIC=0 \
		-DNO_REG_NOSPEC \
		-DSIZE_T_MAX=UINT_MAX \
		-D"__RCSID(x)=" \
		-D'getprogname()="grep"'


LOCAL_SHARED_LIBRARIES := libz

# Make grep launcher for egrep zgrep zegrep.
#
SYMLINKS := $(addprefix $(TARGET_OUT_EXECUTABLES)/,$(PSEUDONYMS))
$(SYMLINKS): GREP_BINARY := $(LOCAL_MODULE)
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(GREP_BINARY) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> $(GREP_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(GREP_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS) $(LOCAL_MODULE)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)


include $(BUILD_EXECUTABLE)
