LOCAL_PATH:= $(call my-dir)

COMMON_LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/bin

include $(CLEAR_VARS)

TARGET_RSA_HOST_KEY := $(TARGET_OUT_ETC)/rsa_host_key
$(eval $(call copy-one-file,$(LOCAL_PATH)/sample_rsa_host_key,$(TARGET_RSA_HOST_KEY)))

LOCAL_ADDITIONAL_DEPENDENCIES:= $(TARGET_RSA_HOST_KEY)

LOCAL_SRC_FILES:=\
	dbutil.c buffer.c \
	dss.c bignum.c \
	signkey.c rsa.c dbrandom.c \
	queue.c \
	atomicio.c compat.c fake-rfc2553.c \
	ltc_prng.c ecc.c ecdsa.c crypto_desc.c \
	gensignkey.c gendss.c genrsa.c

LOCAL_SRC_FILES+=\
	svr-kex.c svr-auth.c sshpty.c \
	svr-authpasswd.c svr-authpubkey.c svr-authpubkeyoptions.c svr-session.c svr-service.c \
	svr-chansession.c svr-runopts.c svr-agentfwd.c svr-main.c svr-x11fwd.c\
	svr-tcpfwd.c svr-authpam.c

LOCAL_SRC_FILES+=\
	cli-main.c cli-auth.c cli-authpasswd.c cli-kex.c \
	cli-session.c cli-runopts.c cli-chansession.c \
	cli-authpubkey.c cli-tcpfwd.c cli-channel.c cli-authinteract.c \
	cli-agentfwd.c

LOCAL_SRC_FILES+=\
	common-session.c packet.c common-algo.c common-kex.c \
	common-channel.c common-chansession.c termcodes.c loginrec.c \
	tcp-accept.c listener.c process-packet.c \
	common-runopts.c circbuffer.c curve25519-donna.c list.c netio.c

LOCAL_SRC_FILES+=dropbearkey.c
LOCAL_SRC_FILES+=scp.c progressmeter.c scpmisc.c
LOCAL_SRC_FILES+=dbmulti.c
LOCAL_SRC_FILES+=dbhelpers.c dh_groups.c

LOCAL_STATIC_LIBRARIES := libtommath libtomcrypt

LOCAL_MODULE_PATH := $(COMMON_LOCAL_MODULE_PATH)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := dropbearmulti
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libtommath
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libtomcrypt/src/headers
LOCAL_CFLAGS +=\
	-DDBMULTI_dropbear -DDBMULTI_dbclient -DDBMULTI_dropbearkey\
	-DDBMULTI_scp -DDROPBEAR_MULTI -DDROPBEAR_SERVER -DDROPBEAR_CLIENT \
	-DPROGRESS_METER -DHAVE_CRYPT
LOCAL_LDLIBS := -lutil -lcrypt

ALL_TOOLS := \
	scp ssh sshd sshd_keys

include $(BUILD_EXECUTABLE)

# Make #!/system/bin/dropbearmulti launchers for each tool.
#
SYMLINKS := $(addprefix $(TARGET_OUT)/usr/bin/,$(ALL_TOOLS))
$(SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk
	@echo "Symlink: $@ -> dropbearmulti"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf dropbearmulti $@

# Create one module that will wrap all the symlinks.  This module requires the
# dropbear module, then adds an explicit dependency on all symlinks.
include $(CLEAR_VARS)
LOCAL_MODULE := dropbear_symlinks
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := dropbear
include $(BUILD_PHONY_PACKAGE)
$(LOCAL_BUILT_MODULE): $(SYMLINKS)

include $(call all-makefiles-under,$(LOCAL_PATH))
