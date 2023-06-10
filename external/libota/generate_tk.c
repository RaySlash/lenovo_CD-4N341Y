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

#include "generate_tk.h"
#include "sha256.h"
#include "encode.h"

#define SHA256 0
#define SHA224 1

#define USE_LOCAL_KEY 0
#define _DEBUG 0


int generate_tk(deviceInfo *devInfo, char *tk, char *prk, char *usr_did,char* usr_m)
{

	char *jwt;
	size_t jwt_length;

	int bytes, ret;

	if (prk == NULL) {
		return -1;
	}

	char did_str[1024] = {0};
	char *did = &did_str[0];

	unsigned char ssn_str[128] = {0};
	unsigned char *ssn = &ssn_str[0];

	unsigned char sm_str[128] = {0};
	unsigned char *sm = &sm_str[0];

	unsigned char sn_dec[128] = {0};
	unsigned char sn_enc[32] = {0};

	unsigned char m_dec[128] = {0};
	unsigned char m_enc[32] = {0};

	strncpy_st(sn_dec, 128,devInfo->deviceSN,128);
	strncpy_st(m_dec, 128,devInfo->m,128);

	mbedtls_sha256_context sha1_ctx;

	mbedtls_sha256_init(&sha1_ctx);
	mbedtls_sha256_starts(&sha1_ctx, SHA256);
	mbedtls_sha256_update(&sha1_ctx, sn_dec, strnlen((char *)sn_dec,128));
	mbedtls_sha256_finish(&sha1_ctx, sn_enc);

	mbedtls_sha256_starts(&sha1_ctx, SHA256);
	mbedtls_sha256_update(&sha1_ctx, m_dec, strnlen((char *)m_dec,128));
	mbedtls_sha256_finish(&sha1_ctx, m_enc);

	for (int i = 0; i < 32; i++) {
		sprintf(ssn+i*2, "%02x", sn_enc[i]);
	}

	for (int k = 0; k < 32; k++) {
		sprintf(sm+k*2, "%02x", m_enc[k]);
	}

	sprintf(did, "%s.%s.null", ssn, sm);
	did[strnlen(ssn,129)+strnlen(sm,129)+strnlen("null",5)+2*strnlen(".",2)] = '\0';


	strncpy_st(usr_m,128,sm,128);
	strncpy_st(usr_did,512, did,512);
	mbedtls_sha256_free(&sha1_ctx);


    struct l8w8jwt_claim header_claims[] = {

	{
	    .key = "kid",
	    .key_length = 3,
	    .value = "some-key-id-here-012345",
	    .value_length = strnlen("some-key-id-here-012345",24),
	    .type = L8W8JWT_CLAIM_TYPE_STRING
	}
    };

    struct l8w8jwt_claim payload_claims[] = {

		{
	    .key = "did",
	    .key_length = 3,
	    .value = did,
	    //.value = "c775e7b757ede630cd0aa1113bd102661ab38829ca52a6422ab782862f268646.67a6b4033b1d60c981dbc055984c5363c43222a99272178fd2de2f49fc73de42.null",
	    .value_length = strnlen(did,1024),
	    .type = L8W8JWT_CLAIM_TYPE_STRING
		},
		{
			.key = "ctx",
			.key_length = 3,
			.value = "Unforseen Consequences",
			.value_length = strnlen("Unforseen Consequences",23),
			.type = L8W8JWT_CLAIM_TYPE_STRING
		},
		{
			.key = "age",
			.key_length = 3,
			.value = "27",
			.value_length = strnlen("27",3),
			.type = L8W8JWT_CLAIM_TYPE_INTEGER
		},
		{
			.key = "size",
			.key_length = strnlen("size",5),
			.value = "1.85",
			.value_length = strnlen("1.85",5),
			.type = L8W8JWT_CLAIM_TYPE_NUMBER
			},
		{
			.key = "alive",
			.key_length = strnlen("alive",6),
			.value = "true",
			.value_length = strnlen("true",5),
			.type = L8W8JWT_CLAIM_TYPE_BOOLEAN
		},
		{
			.key = "nulltest",
			.key_length = strnlen("nulltest",9),
			.value = "null",
			.value_length = strnlen("null",5),
			.type = L8W8JWT_CLAIM_TYPE_NULL
		}
	};

	struct l8w8jwt_encoding_params params;

	l8w8jwt_encoding_params_init(&params);

	params.alg = L8W8JWT_ALG_ES256;

	params.sub = "Gordon Freeman";
	params.sub_length = strnlen("Gordon Freeman",15);

	params.iss = "Black Mesa";
	params.iss_length = strnlen("Black Mesa",11);

	params.aud = "Administrator";
	params.aud_length = strnlen("Administrator",14);

	params.iat = time(NULL);
	params.exp = time(NULL) + 600; // Set to expire after 10 minutes (600 seconds).

	params.additional_header_claims = header_claims;
	params.additional_header_claims_count = sizeof(header_claims) / sizeof(struct l8w8jwt_claim);

	params.additional_payload_claims = payload_claims;
	params.additional_payload_claims_count = sizeof(payload_claims) / sizeof(struct l8w8jwt_claim);


	params.secret_key = (unsigned char *)prk;
	params.secret_key_length = strnlen(prk,256);

	params.out = &jwt;
	params.out_length = &jwt_length;

	int r = l8w8jwt_encode(&params);

	strncpy_st(tk,1024, jwt,1024);
	if (r == L8W8JWT_SUCCESS)
		ret = 0;
	else
		ret = -1;


	free(jwt); /* Never forget to free the jwt string! */
	return ret;
}

int generate_k(char *pr, char *pu)
{
	int ret, bytes;
	char pr_v[256] = {0};
	char pu_v[256] = {0};

	ret = system(" openssl ecparam -name prime256v1 -genkey -noout -out /tmp/pr.pem && openssl ec -in /tmp/pr.pem -pubout -out /tmp/pu.pem ");
	ret = system("cat /tmp/pu.pem  | busybox awk 'NR>2{print p}{p=$0}' > /tmp/final_pu.pem");

	int prifd = open("/tmp/pr.pem", O_RDWR | O_CREAT, 0755);

	if (prifd < 0) {
		return -1;
	}

	bytes = read(prifd, pr_v, sizeof(pr_v));
	if (bytes <= 0) {
		close(prifd);
		return -2;
	}

	int pubfd = open("/tmp/final_pu.pem", O_RDWR | O_CREAT, 0755);

	if (pubfd < 0) {
                close(prifd);
		return -3;
	}

	bytes = read(pubfd, pu_v, sizeof(pu_v));
	if (bytes <= 0) {
		close(pubfd);
                close(prifd);
		return -4;
	}

	close(prifd);
	close(pubfd);

	strncpy_st(pr,256, pr_v,256);
	strncpy_st(pu,256, pu_v,256);

	system("rm /tmp/final_pu.pem");
	system("rm /tmp/pr.pem");

	return 0;

}
