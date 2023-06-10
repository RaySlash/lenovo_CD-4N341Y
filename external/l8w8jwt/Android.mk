# vendor/amlogic/common/fota/Android.mk
#
# Copyright (C) 2019 Amlogic, Inc. All rights reserved.
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libjwt

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_C_FILES := $(LOCAL_PATH)/src/$(call all-c-files-under)

LOCAL_C_INCLUDES :=  \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/lib/chillbuff/include/ \
    $(LOCAL_PATH)/lib/mbedtls/include \
    $(LOCAL_PATH)/lib/jsmn/ \
    $(LOCAL_PATH)/lib/checknum/include/

#examples/es256/encode.c
LOCAL_SRC_FILES := \
    src/encode.c \
    src/decode.c \
    src/claim.c \
    src/base64.c \
    lib/mbedtls/library/pk.c \
    lib/mbedtls/library/pkparse.c \
    lib/mbedtls/library/entropy.c \
    lib/mbedtls/library/sha256.c \
    lib/mbedtls/library/ctr_drbg.c \
    lib/mbedtls/library/md_wrap.c \
    lib/mbedtls/library/sha512.c \
    lib/mbedtls/library/md.c \
    lib/mbedtls/library/ecdsa.c \
    lib/mbedtls/library/pk_wrap.c \
    lib/mbedtls/library/pem.c \
    lib/mbedtls/library/bignum.c \
    lib/mbedtls/library/rsa.c \
    lib/mbedtls/library/platform_util.c \
    lib/mbedtls/library/oid.c \
    lib/mbedtls/library/asn1parse.c \
    lib/mbedtls/library/ecp.c \
    lib/mbedtls/library/md5.c \
    lib/mbedtls/library/rsa_internal.c \
    lib/mbedtls/library/ecp_curves.c \
    lib/mbedtls/library/pkcs12.c \
    lib/mbedtls/library/aes.c \
    lib/mbedtls/library/cipher.c \
    lib/mbedtls/library/arc4.c \
    lib/mbedtls/library/base64.c \
    lib/mbedtls/library/des.c \
    lib/mbedtls/library/asn1write.c \
    lib/mbedtls/library/hmac_drbg.c \
    lib/mbedtls/library/sha1.c \
    lib/mbedtls/library/pkcs5.c \
    lib/mbedtls/library/entropy_poll.c \
    lib/mbedtls/library/ripemd160.c \
    lib/mbedtls/library/cipher_wrap.c \
    lib/mbedtls/library/chacha20.c \
    lib/mbedtls/library/gcm.c \
    lib/mbedtls/library/chachapoly.c \
    lib/mbedtls/library/timing.c \
    lib/mbedtls/library/ccm.c \
    lib/mbedtls/library/blowfish.c \
    lib/mbedtls/library/camellia.c \
    lib/mbedtls/library/poly1305.c

include $(BUILD_STATIC_LIBRARY)
