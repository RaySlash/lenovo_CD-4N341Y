LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libtomcrypt
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/headers $(LOCAL_PATH)/.. $(LOCAL_PATH)/../libtommath

LOCAL_WHOLE_STATIC_LIBRARIES := libtommath

LOCAL_CFLAGS += -DLTC_SOURCE
LOCAL_SRC_FILES := \
src/ciphers/aes/aes.c src/ciphers/anubis.c src/ciphers/blowfish.c \
src/ciphers/cast5.c src/ciphers/des.c src/ciphers/kasumi.c src/ciphers/khazad.c src/ciphers/kseed.c \
src/ciphers/noekeon.c src/ciphers/rc2.c src/ciphers/rc5.c src/ciphers/rc6.c src/ciphers/safer/safer.c \
src/ciphers/safer/safer_tab.c src/ciphers/safer/saferp.c src/ciphers/skipjack.c \
src/ciphers/twofish/twofish.c src/ciphers/xtea.c src/encauth/ccm/ccm_memory.c \
src/encauth/ccm/ccm_test.c src/encauth/eax/eax_addheader.c src/encauth/eax/eax_decrypt.c \
src/encauth/eax/eax_decrypt_verify_memory.c src/encauth/eax/eax_done.c src/encauth/eax/eax_encrypt.c \
src/encauth/eax/eax_encrypt_authenticate_memory.c src/encauth/eax/eax_init.c \
src/encauth/eax/eax_test.c src/encauth/gcm/gcm_add_aad.c src/encauth/gcm/gcm_add_iv.c \
src/encauth/gcm/gcm_done.c src/encauth/gcm/gcm_gf_mult.c src/encauth/gcm/gcm_init.c \
src/encauth/gcm/gcm_memory.c src/encauth/gcm/gcm_mult_h.c src/encauth/gcm/gcm_process.c \
src/encauth/gcm/gcm_reset.c src/encauth/gcm/gcm_test.c src/encauth/ocb/ocb_decrypt.c \
src/encauth/ocb/ocb_decrypt_verify_memory.c src/encauth/ocb/ocb_done_decrypt.c \
src/encauth/ocb/ocb_done_encrypt.c src/encauth/ocb/ocb_encrypt.c \
src/encauth/ocb/ocb_encrypt_authenticate_memory.c src/encauth/ocb/ocb_init.c src/encauth/ocb/ocb_ntz.c \
src/encauth/ocb/ocb_shift_xor.c src/encauth/ocb/ocb_test.c src/encauth/ocb/s_ocb_done.c \
src/hashes/chc/chc.c src/hashes/helper/hash_file.c src/hashes/helper/hash_filehandle.c \
src/hashes/helper/hash_memory.c src/hashes/helper/hash_memory_multi.c src/hashes/md2.c src/hashes/md4.c \
src/hashes/md5.c src/hashes/rmd128.c src/hashes/rmd160.c src/hashes/rmd256.c src/hashes/rmd320.c \
src/hashes/sha1.c src/hashes/sha2/sha256.c src/hashes/sha2/sha512.c src/hashes/tiger.c \
src/hashes/whirl/whirl.c src/mac/f9/f9_done.c src/mac/f9/f9_file.c src/mac/f9/f9_init.c \
src/mac/f9/f9_memory.c src/mac/f9/f9_memory_multi.c src/mac/f9/f9_process.c src/mac/f9/f9_test.c \
src/mac/hmac/hmac_done.c src/mac/hmac/hmac_file.c src/mac/hmac/hmac_init.c src/mac/hmac/hmac_memory.c \
src/mac/hmac/hmac_memory_multi.c src/mac/hmac/hmac_process.c src/mac/hmac/hmac_test.c \
src/mac/omac/omac_done.c src/mac/omac/omac_file.c src/mac/omac/omac_init.c src/mac/omac/omac_memory.c \
src/mac/omac/omac_memory_multi.c src/mac/omac/omac_process.c src/mac/omac/omac_test.c \
src/mac/pelican/pelican.c src/mac/pelican/pelican_memory.c src/mac/pelican/pelican_test.c \
src/mac/pmac/pmac_done.c src/mac/pmac/pmac_file.c src/mac/pmac/pmac_init.c src/mac/pmac/pmac_memory.c \
src/mac/pmac/pmac_memory_multi.c src/mac/pmac/pmac_ntz.c src/mac/pmac/pmac_process.c \
src/mac/pmac/pmac_shift_xor.c src/mac/pmac/pmac_test.c src/mac/xcbc/xcbc_done.c \
src/mac/xcbc/xcbc_file.c src/mac/xcbc/xcbc_init.c src/mac/xcbc/xcbc_memory.c \
src/mac/xcbc/xcbc_memory_multi.c src/mac/xcbc/xcbc_process.c src/mac/xcbc/xcbc_test.c \
src/math/fp/ltc_ecc_fp_mulmod.c src/math/gmp_desc.c src/math/ltm_desc.c src/math/multi.c \
src/math/rand_prime.c src/math/tfm_desc.c src/misc/base64/base64_decode.c \
src/misc/base64/base64_encode.c src/misc/burn_stack.c src/misc/crypt/crypt.c \
src/misc/crypt/crypt_argchk.c src/misc/crypt/crypt_cipher_descriptor.c \
src/misc/crypt/crypt_cipher_is_valid.c src/misc/crypt/crypt_find_cipher.c \
src/misc/crypt/crypt_find_cipher_any.c src/misc/crypt/crypt_find_cipher_id.c \
src/misc/crypt/crypt_find_hash.c src/misc/crypt/crypt_find_hash_any.c \
src/misc/crypt/crypt_find_hash_id.c src/misc/crypt/crypt_find_hash_oid.c \
src/misc/crypt/crypt_find_prng.c src/misc/crypt/crypt_fsa.c src/misc/crypt/crypt_hash_descriptor.c \
src/misc/crypt/crypt_hash_is_valid.c src/misc/crypt/crypt_ltc_mp_descriptor.c \
src/misc/crypt/crypt_prng_descriptor.c src/misc/crypt/crypt_prng_is_valid.c \
src/misc/crypt/crypt_register_cipher.c src/misc/crypt/crypt_register_hash.c \
src/misc/crypt/crypt_register_prng.c src/misc/crypt/crypt_unregister_cipher.c \
src/misc/crypt/crypt_unregister_hash.c src/misc/crypt/crypt_unregister_prng.c \
src/misc/error_to_string.c src/misc/pkcs5/pkcs_5_1.c src/misc/pkcs5/pkcs_5_2.c src/misc/zeromem.c \
src/modes/cbc/cbc_decrypt.c src/modes/cbc/cbc_done.c src/modes/cbc/cbc_encrypt.c \
src/modes/cbc/cbc_getiv.c src/modes/cbc/cbc_setiv.c src/modes/cbc/cbc_start.c \
src/modes/cfb/cfb_decrypt.c src/modes/cfb/cfb_done.c src/modes/cfb/cfb_encrypt.c \
src/modes/cfb/cfb_getiv.c src/modes/cfb/cfb_setiv.c src/modes/cfb/cfb_start.c \
src/modes/ctr/ctr_decrypt.c src/modes/ctr/ctr_done.c src/modes/ctr/ctr_encrypt.c \
src/modes/ctr/ctr_getiv.c src/modes/ctr/ctr_setiv.c src/modes/ctr/ctr_start.c src/modes/ctr/ctr_test.c \
src/modes/ecb/ecb_decrypt.c src/modes/ecb/ecb_done.c src/modes/ecb/ecb_encrypt.c \
src/modes/ecb/ecb_start.c src/modes/f8/f8_decrypt.c src/modes/f8/f8_done.c src/modes/f8/f8_encrypt.c \
src/modes/f8/f8_getiv.c src/modes/f8/f8_setiv.c src/modes/f8/f8_start.c src/modes/f8/f8_test_mode.c \
src/modes/lrw/lrw_decrypt.c src/modes/lrw/lrw_done.c src/modes/lrw/lrw_encrypt.c \
src/modes/lrw/lrw_getiv.c src/modes/lrw/lrw_process.c src/modes/lrw/lrw_setiv.c \
src/modes/lrw/lrw_start.c src/modes/lrw/lrw_test.c src/modes/ofb/ofb_decrypt.c src/modes/ofb/ofb_done.c \
src/modes/ofb/ofb_encrypt.c src/modes/ofb/ofb_getiv.c src/modes/ofb/ofb_setiv.c \
src/modes/ofb/ofb_start.c src/pk/asn1/der/bit/der_decode_bit_string.c \
src/pk/asn1/der/bit/der_encode_bit_string.c src/pk/asn1/der/bit/der_length_bit_string.c \
src/pk/asn1/der/boolean/der_decode_boolean.c src/pk/asn1/der/boolean/der_encode_boolean.c \
src/pk/asn1/der/boolean/der_length_boolean.c src/pk/asn1/der/choice/der_decode_choice.c \
src/pk/asn1/der/ia5/der_decode_ia5_string.c src/pk/asn1/der/ia5/der_encode_ia5_string.c \
src/pk/asn1/der/ia5/der_length_ia5_string.c src/pk/asn1/der/integer/der_decode_integer.c \
src/pk/asn1/der/integer/der_encode_integer.c src/pk/asn1/der/integer/der_length_integer.c \
src/pk/asn1/der/object_identifier/der_decode_object_identifier.c \
src/pk/asn1/der/object_identifier/der_encode_object_identifier.c \
src/pk/asn1/der/object_identifier/der_length_object_identifier.c \
src/pk/asn1/der/octet/der_decode_octet_string.c src/pk/asn1/der/octet/der_encode_octet_string.c \
src/pk/asn1/der/octet/der_length_octet_string.c \
src/pk/asn1/der/printable_string/der_decode_printable_string.c \
src/pk/asn1/der/printable_string/der_encode_printable_string.c \
src/pk/asn1/der/printable_string/der_length_printable_string.c \
src/pk/asn1/der/sequence/der_decode_sequence_ex.c \
src/pk/asn1/der/sequence/der_decode_sequence_flexi.c \
src/pk/asn1/der/sequence/der_decode_sequence_multi.c \
src/pk/asn1/der/sequence/der_encode_sequence_ex.c \
src/pk/asn1/der/sequence/der_encode_sequence_multi.c src/pk/asn1/der/sequence/der_length_sequence.c \
src/pk/asn1/der/sequence/der_sequence_free.c src/pk/asn1/der/set/der_encode_set.c \
src/pk/asn1/der/set/der_encode_setof.c src/pk/asn1/der/short_integer/der_decode_short_integer.c \
src/pk/asn1/der/short_integer/der_encode_short_integer.c \
src/pk/asn1/der/short_integer/der_length_short_integer.c src/pk/asn1/der/utctime/der_decode_utctime.c \
src/pk/asn1/der/utctime/der_encode_utctime.c src/pk/asn1/der/utctime/der_length_utctime.c \
src/pk/asn1/der/utf8/der_decode_utf8_string.c src/pk/asn1/der/utf8/der_encode_utf8_string.c \
src/pk/asn1/der/utf8/der_length_utf8_string.c src/pk/dsa/dsa_decrypt_key.c \
src/pk/dsa/dsa_encrypt_key.c src/pk/dsa/dsa_export.c src/pk/dsa/dsa_free.c src/pk/dsa/dsa_import.c \
src/pk/dsa/dsa_make_key.c src/pk/dsa/dsa_shared_secret.c src/pk/dsa/dsa_sign_hash.c \
src/pk/dsa/dsa_verify_hash.c src/pk/dsa/dsa_verify_key.c src/pk/ecc/ecc.c \
src/pk/ecc/ecc_ansi_x963_export.c src/pk/ecc/ecc_ansi_x963_import.c src/pk/ecc/ecc_decrypt_key.c \
src/pk/ecc/ecc_encrypt_key.c src/pk/ecc/ecc_export.c src/pk/ecc/ecc_free.c src/pk/ecc/ecc_get_size.c \
src/pk/ecc/ecc_import.c src/pk/ecc/ecc_make_key.c src/pk/ecc/ecc_shared_secret.c \
src/pk/ecc/ecc_sign_hash.c src/pk/ecc/ecc_sizes.c src/pk/ecc/ecc_test.c src/pk/ecc/ecc_verify_hash.c \
src/pk/ecc/ltc_ecc_is_valid_idx.c src/pk/ecc/ltc_ecc_map.c src/pk/ecc/ltc_ecc_mul2add.c \
src/pk/ecc/ltc_ecc_mulmod.c src/pk/ecc/ltc_ecc_mulmod_timing.c src/pk/ecc/ltc_ecc_points.c \
src/pk/ecc/ltc_ecc_projective_add_point.c src/pk/ecc/ltc_ecc_projective_dbl_point.c \
src/pk/katja/katja_decrypt_key.c src/pk/katja/katja_encrypt_key.c src/pk/katja/katja_export.c \
src/pk/katja/katja_exptmod.c src/pk/katja/katja_free.c src/pk/katja/katja_import.c \
src/pk/katja/katja_make_key.c src/pk/pkcs1/pkcs_1_i2osp.c src/pk/pkcs1/pkcs_1_mgf1.c \
src/pk/pkcs1/pkcs_1_oaep_decode.c src/pk/pkcs1/pkcs_1_oaep_encode.c src/pk/pkcs1/pkcs_1_os2ip.c \
src/pk/pkcs1/pkcs_1_pss_decode.c src/pk/pkcs1/pkcs_1_pss_encode.c src/pk/pkcs1/pkcs_1_v1_5_decode.c \
src/pk/pkcs1/pkcs_1_v1_5_encode.c src/pk/rsa/rsa_decrypt_key.c src/pk/rsa/rsa_encrypt_key.c \
src/pk/rsa/rsa_export.c src/pk/rsa/rsa_exptmod.c src/pk/rsa/rsa_free.c src/pk/rsa/rsa_import.c \
src/pk/rsa/rsa_make_key.c src/pk/rsa/rsa_sign_hash.c src/pk/rsa/rsa_verify_hash.c src/prngs/fortuna.c \
src/prngs/rc4.c src/prngs/rng_get_bytes.c src/prngs/rng_make_prng.c src/prngs/sober128.c \
src/prngs/sprng.c src/prngs/yarrow.c

include $(BUILD_STATIC_LIBRARY)
