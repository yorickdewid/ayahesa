/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include <ayahesa.h>

#include <openssl/x509.h>
#include <openssl/hmac.h>
#include <time.h>

#include "../core/base64url.h"

#define TEST_JTI    "3713bf43-4490-42c5-855d-6563a2755ffe"

char *jwt_token_new(const char *key, const char *issuer, const char *subject, const char *audience);

char *
jwt_token_new(const char *key, const char *issuer, const char *subject, const char *audience)
{
    /* Hard coded header */
    char *header_encoded = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
    size_t header_encoded_length = 36;

    yajl_gen jwt = yajl_gen_alloc(NULL);

    yajl_gen_map_open(jwt);
    yajl_gen_string(jwt, (const unsigned char *)"iss", 3);
    yajl_gen_string(jwt, (const unsigned char *)issuer, strlen(issuer));
    yajl_gen_string(jwt, (const unsigned char *)"sub", 3);
    yajl_gen_string(jwt, (const unsigned char *)subject, strlen(subject));
    yajl_gen_string(jwt, (const unsigned char *)"aud", 3);
    yajl_gen_string(jwt, (const unsigned char *)audience, strlen(audience));
    yajl_gen_string(jwt, (const unsigned char *)"iat", 3);
    yajl_gen_integer(jwt, (long long int)time(NULL));
    yajl_gen_string(jwt, (const unsigned char *)"exp", 3);
    yajl_gen_integer(jwt, (long long int)time(NULL) + 3600);
    yajl_gen_string(jwt, (const unsigned char *)"jti", 3);
    yajl_gen_string(jwt, (const unsigned char *)TEST_JTI, strlen(TEST_JTI));
    yajl_gen_map_close(jwt);

    /* Object to string */
    const unsigned char *payload;
    size_t payload_len;
    yajl_gen_get_buf(jwt, &payload, &payload_len);
    
    /* String encode */
    unsigned char *_payload_encoded = (unsigned char *)kore_malloc(((4 * payload_len) / 3 ) + 2);
    size_t payload_encoded_length;
    unsigned char *payload_encoded = base64url_encode(payload, payload_len, _payload_encoded, &payload_encoded_length);

    yajl_gen_clear(jwt);
    yajl_gen_free(jwt);

    /* Prepare signature input */
    char *data = (char *)kore_malloc(header_encoded_length + 1 + payload_encoded_length + 1);
    sprintf(data, "%s.%s", header_encoded, payload_encoded);

    unsigned int signature_len;
    unsigned char signature[EVP_MAX_MD_SIZE];
    HMAC(EVP_sha256(),
         (const unsigned char *)key, strlen(key),
         (const unsigned char *)data, strlen(data),
         signature, &signature_len);

    kore_free(data);

    /* Convert signature to base64 */
    unsigned char *_signature_encoded = (unsigned char *)kore_malloc(((4 * signature_len) / 3 ) + 2);
    size_t signature_encoded_length;
    unsigned char *signature_encoded = base64url_encode(signature, signature_len, _signature_encoded, &signature_encoded_length);
    signature_encoded[signature_encoded_length] = '\0';

    char *token = (char *)kore_malloc(header_encoded_length + 1 + payload_encoded_length + 1 + signature_encoded_length + 1);
    sprintf(token, "%s.%s.%s", header_encoded, payload_encoded, signature_encoded);

    kore_free(_signature_encoded);
    kore_free(_payload_encoded);

    return token;
}
