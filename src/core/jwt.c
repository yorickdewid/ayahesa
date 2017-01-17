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

#include "base64url.h"

#define TEST_JTI    "3713bf43-4490-42c5-855d-6563a2755ffe"

static unsigned char *
jwt_payload(const char *issuer, const char *subject, const char *audience, size_t *payload_encoded_length)
{
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
    unsigned char *payload_encoded = (unsigned char *)kore_calloc(((4 * payload_len) / 3 ) + 2, sizeof(unsigned char));
    base64url_encode(payload, payload_len, payload_encoded, payload_encoded_length);

    yajl_gen_clear(jwt);
    yajl_gen_free(jwt);

    return payload_encoded;
}

static unsigned char *
jwt_sign(const char *key, const char *data, size_t *signature_encoded_length)
{
    unsigned int signature_len;
    unsigned char signature[EVP_MAX_MD_SIZE];

    HMAC(EVP_sha256(),
         (const unsigned char *)key, strlen(key),
         (const unsigned char *)data, strlen(data),
         signature, &signature_len);

    /* Signature encode */
    unsigned char *signature_encoded = (unsigned char *)kore_calloc(((4 * signature_len) / 3 ) + 2, sizeof(unsigned char));
    base64url_encode(signature, signature_len, signature_encoded, signature_encoded_length);

    return signature_encoded;
}

char *
jwt_token_new(const char *key, const char *issuer, const char *subject, const char *audience)
{
    /* Hard coded header */
    unsigned char *header_encoded = (unsigned char *)"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
    unsigned char *payload_encoded = NULL;
    unsigned char *signature_encoded = NULL;
    size_t header_encoded_length = 36;
    size_t payload_encoded_length;
    size_t signature_encoded_length;

    /* Generate payload */
    payload_encoded = jwt_payload(issuer, subject, audience, &payload_encoded_length);

    /*
     * Prepare signature input
     * Generate signature over data
     */
    char *data = (char *)kore_calloc(header_encoded_length + 1 + payload_encoded_length + 1, sizeof(unsigned char));
    sprintf(data, "%s.%s", header_encoded, payload_encoded);
    signature_encoded = jwt_sign(key, data, &signature_encoded_length);
    kore_free(data);

    /* Concat parts */
    char *token = (char *)kore_calloc(header_encoded_length + 1 + payload_encoded_length + 1 + signature_encoded_length + 1, sizeof(unsigned char));
    sprintf(token, "%s.%s.%s", header_encoded, payload_encoded, signature_encoded);

    kore_free(signature_encoded);
    kore_free(payload_encoded);

    return token;
}
