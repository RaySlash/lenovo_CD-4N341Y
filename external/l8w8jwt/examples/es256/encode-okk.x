/*
   Copyright 2020 Raphael Beck

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mbedtls/sha256.h"
#include "l8w8jwt/encode.h"

#define SHA256 0
#define SHA224 1

/*
 * This keypair was generated using the following command:
 * openssl ecparam -name prime256v1 -genkey -noout -out private.pem && openssl ec -in private.pem -pubout -out public.pem
 */

#if 1
#if 0
static const char ECDSA_PRIVATE_KEY[] = "-----BEGIN EC PRIVATE KEY-----\n"
                                        "MHcCAQEEILvM6E7mLOdndALDyFc3sOgUTb6iVjgwRBtBwYZngSuwoAoGCCqGSM49\n"
                                        "AwEHoUQDQgAEMlFGAIxe+/zLanxz4bOxTI6daFBkNGyQ+P4bc/RmNEq1NpsogiMB\n"
                                        "5eXC7jUcD/XqxP9HCIhdRBcQHx7aOo3ayQ=0\n"
                                        "-----END EC PRIVATE KEY-----";
#endif
#if 1
static const char ECDSA_PRIVATE_KEY[] = "-----BEGIN EC PRIVATE KEY-----\n"
"MHcCAQEEIOHFJzxekZeiVbHMgJeNTXs8vKdR21KkHIi6n7153oJFoAoGCCqGSM49"
"AwEHoUQDQgAER9Y0a5yuHT0ieqqWCBkWClS5ZzodNORf3H5IB0XgoZ8jfL0gxYFs"
"zMpQvE4t5q02IEwnlGLDRXSm608ix2sREQ=="
"-----END EC PRIVATE KEY-----";
#endif


static const char ECDSA_PUBLIC_KEY[] = "-----BEGIN PUBLIC KEY-----\n"
"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAErz/GZj6AjoJY6+nUC0+yJ6FA9OoZ\n"
"k2h0bFhh7GaGJz4c1o6ZorEVRmPmZC1rXgJvlz9zdzIw5I7pLoFGwgm2NA==\n"
"-----END PUBLIC KEY-----";
#endif

unsigned char CharToHex(unsigned char bHex){
if((bHex>=0)&&(bHex<=9))
bHex += 0x30;
else if((bHex>=10)&&(bHex<=15))//´óÐ´×ÖÄ¸
bHex += 0x37;
else bHex = 0xff;
return bHex;
}
#if 1
char* sh256_create_sn(void) {
	
		unsigned char *ssn;
		unsigned char sn_encrypt[] = "1234567890";
		unsigned char sn_decrypt[512];
		
		
		mbedtls_sha256_context sha1_ctx_sn;
		mbedtls_sha256_init(&sha1_ctx_sn);
		mbedtls_sha256_starts(&sha1_ctx_sn,SHA256);
		mbedtls_sha256_update(&sha1_ctx_sn, sn_encrypt, strlen((char *)sn_encrypt));
		mbedtls_sha256_finish(&sha1_ctx_sn, sn_decrypt);
		mbedtls_sha256_free(&sha1_ctx_sn);
		
		ssn = (unsigned char*)malloc(sizeof(unsigned char)*512);
		printf(" len: %d \n ",strlen((unsigned char *)sn_decrypt));
	
		for (int i = 0; i< strlen((unsigned char *)sn_decrypt) - 4; i++) {
			sprintf(ssn+i*2,"%02x", sn_decrypt[i]);
		}
		ssn[strlen(ssn)+1] = '\0';
		
		printf(" SN:\n%s\n ",ssn);
		return NULL;


}

char* sh256_create_mac(void) {
	unsigned char *smac;
	unsigned char mac_encrypt[] = "ac:83:f3:b4:82:de";
	unsigned char mac_decrypt[512];
	
	smac = (unsigned char*)malloc(sizeof(unsigned char)*512);
	
	mbedtls_sha256_context sha1_ctx_mac;
	mbedtls_sha256_init(&sha1_ctx_mac);
	mbedtls_sha256_starts(&sha1_ctx_mac,SHA256);
	mbedtls_sha256_update(&sha1_ctx_mac, mac_encrypt, strlen((char *)mac_encrypt));
    mbedtls_sha256_finish(&sha1_ctx_mac, mac_decrypt);
	mbedtls_sha256_free(&sha1_ctx_mac);

	for (int k = 0; k< strlen((unsigned char *)mac_decrypt) - 1; k++) {
        sprintf(smac+k*2,"%02x", mac_decrypt[k]);
    }
	smac[strlen(smac)+1] = '\0';

	printf(" MAC:\n%s\n ",smac);
	return NULL;
}

#endif
int main(void)
{
    char* jwt;
    size_t jwt_length;

	int bytes,ret;
	char prikey[512];
	
	//sh256_create_sn();
	//sh256_create_mac();
	#if 1
			unsigned char *ssn;
		unsigned char sn_encrypt[] = "1234567890";
		unsigned char sn_decrypt[512];

		unsigned char *smac;
	unsigned char mac_encrypt[] = "ac:83:f3:b4:82:de";
	unsigned char mac_decrypt[512];
	
	smac = (unsigned char*)malloc(sizeof(unsigned char)*512);
		
		
		mbedtls_sha256_context sha1_ctx;
		mbedtls_sha256_init(&sha1_ctx);
		mbedtls_sha256_starts(&sha1_ctx,SHA256);
		mbedtls_sha256_update(&sha1_ctx, sn_encrypt, strlen((char *)sn_encrypt));
		mbedtls_sha256_finish(&sha1_ctx, sn_decrypt);

		mbedtls_sha256_starts(&sha1_ctx,SHA256);
		mbedtls_sha256_update(&sha1_ctx, mac_encrypt, strlen((char *)mac_encrypt));
   		mbedtls_sha256_finish(&sha1_ctx, mac_decrypt);
		mbedtls_sha256_free(&sha1_ctx);
		
		ssn = (unsigned char*)malloc(sizeof(unsigned char)*512);
		
	    printf(" len: %d \n ",strlen((unsigned char *)sn_decrypt));
		for (int i = 0; i< strlen((unsigned char *)sn_decrypt) - 1; i++) {
			sprintf(ssn+i*2,"%02x", sn_decrypt[i]);
		}
		ssn[strlen(ssn)+1] = '\0';
		
		printf(" SN:\n%s\n ",ssn);
		
		for (int k = 0; k< strlen((unsigned char *)mac_decrypt) - 1; k++) {
        	sprintf(smac+k*2,"%02x", mac_decrypt[k]);
    	}
		smac[strlen(smac)+1] = '\0';

		printf(" MAC:\n%s\n ",smac);
#endif
	ret = system(" openssl ecparam -name prime256v1 -genkey -noout -out /data/private.pem && openssl ec -in private.pem -pubout -out /data/public.pem ");
	
	int fd = open("/data/private.pem",O_RDWR | O_CREAT,0755);
	if (fd < 0) {
		printf(" create file failed %s \n",strerror(errno));
	}
	bytes = read(fd,prikey,sizeof(prikey));
	if (bytes <= 0) {
		printf(" read private key failed, %s ",strerror(errno));
	}
	

#if 0


	

	mbedtls_sha256_init(&sha1_ctx);
	mbedtls_sha256_starts(&sha1_ctx,SHA256);
	mbedtls_sha256_update(&sha1_ctx, mac_encrypt, strlen((char *)mac_encrypt));
    mbedtls_sha256_finish(&sha1_ctx, mac_decrypt);
	mbedtls_sha256_free(&sha1_ctx);
	
#endif
	

    struct l8w8jwt_claim header_claims[] =
    {
        {
            .key = "kid",
            .key_length = 3,
            .value = "some-key-id-here-012345",
            .value_length = strlen("some-key-id-here-012345"),
            .type = L8W8JWT_CLAIM_TYPE_STRING
        }
    };

    struct l8w8jwt_claim payload_claims[] =
    {
    	{
            .key = "did",
            .key_length = 3,
            .value = "c775e7b757ede630cd0aa1113bd102661ab38829ca52a6422ab782862f268646.67a6b4033b1d60c981dbc055984c5363c43222a99272178fd2de2f49fc73de42.null",
            .value_length = strlen("c775e7b757ede630cd0aa1113bd102661ab38829ca52a6422ab782862f268646.67a6b4033b1d60c981dbc055984c5363c43222a99272178fd2de2f49fc73de42.null"),
            .type = L8W8JWT_CLAIM_TYPE_STRING
        },
        {
            .key = "ctx",
            .key_length = 3,
            .value = "Unforseen Consequences",
            .value_length = strlen("Unforseen Consequences"),
            .type = L8W8JWT_CLAIM_TYPE_STRING
        },
        {
            .key = "age",
            .key_length = 3,
            .value = "27",
            .value_length = strlen("27"),
            .type = L8W8JWT_CLAIM_TYPE_INTEGER
        },
        {
            .key = "size",
            .key_length = strlen("size"),
            .value = "1.85",
            .value_length = strlen("1.85"),
            .type = L8W8JWT_CLAIM_TYPE_NUMBER
        },
        {
            .key = "alive",
            .key_length = strlen("alive"),
            .value = "true",
            .value_length = strlen("true"),
            .type = L8W8JWT_CLAIM_TYPE_BOOLEAN
        },
        {
            .key = "nulltest",
            .key_length = strlen("nulltest"),
            .value = "null",
            .value_length = strlen("null"),
            .type = L8W8JWT_CLAIM_TYPE_NULL
        }
    };

    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_ES256;

    params.sub = "Gordon Freeman";
    params.sub_length = strlen("Gordon Freeman");

    params.iss = "Black Mesa";
    params.iss_length = strlen("Black Mesa");

    params.aud = "Administrator";
    params.aud_length = strlen("Administrator");

    params.iat = time(NULL);
    params.exp = time(NULL) + 600; // Set to expire after 10 minutes (600 seconds).

    params.additional_header_claims = header_claims;
    params.additional_header_claims_count = sizeof(header_claims) / sizeof(struct l8w8jwt_claim);

    params.additional_payload_claims = payload_claims;
    params.additional_payload_claims_count = sizeof(payload_claims) / sizeof(struct l8w8jwt_claim);

    params.secret_key = (unsigned char*)prikey;
    params.secret_key_length = strlen(prikey);

    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);
    printf("\nl8w8jwt_encode_es256 function returned %s (code %d).\n\nCreated token: \n%s\n", r == L8W8JWT_SUCCESS ? "successfully" : "", r, jwt);

    free(jwt); /* Never forget to free the jwt string! */
    return 0;
}
