/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include "../include/ayahesa.h"
#include "../include/quid.h"

#include <openssl/x509.h>
#include <openssl/hmac.h>
#include <time.h>

#include "base64url.h"

#define HEADER_ENCODED  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"

static unsigned char *
generate_payload(const char *issuer, const char *subject, const char *audience, size_t *payload_encoded_length)
{
    yajl_gen jwt = yajl_gen_alloc(NULL);

    cuuid_t jti;
    char jti_str[QUID_FULLLEN + 1];
    quid_create(&jti, IDF_SIGNED | IDF_TAGGED, CLS_CMON, "JWT");
    quid_tostring(&jti, jti_str);

    yajl_gen_map_open(jwt);
    yajl_gen_string(jwt, (const unsigned char *)"iss", 3);
    yajl_gen_string(jwt, (const unsigned char *)issuer, strlen(issuer));
    yajl_gen_string(jwt, (const unsigned char *)"sub", 3);
    yajl_gen_string(jwt, (const unsigned char *)subject, strlen(subject));
    yajl_gen_string(jwt, (const unsigned char *)"oid", 3);
    yajl_gen_string(jwt, (const unsigned char *)"10017", 5);
    yajl_gen_string(jwt, (const unsigned char *)"aud", 3);
    yajl_gen_string(jwt, (const unsigned char *)audience, strlen(audience));
    yajl_gen_string(jwt, (const unsigned char *)"iat", 3);
    yajl_gen_integer(jwt, (long long int)time(NULL));
    yajl_gen_string(jwt, (const unsigned char *)"exp", 3);
    yajl_gen_integer(jwt, (long long int)time(NULL) + app_session_lifetime());
    yajl_gen_string(jwt, (const unsigned char *)"jti", 3);
    yajl_gen_string(jwt, (const unsigned char *)jti_str, QUID_FULLLEN);
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
sign_payload(char *key, size_t key_length, const char *data, size_t *signature_encoded_length)
{
    unsigned int signature_len;
    unsigned char signature[EVP_MAX_MD_SIZE];

    HMAC(EVP_sha256(),
         (const unsigned char *)key, key_length,
         (const unsigned char *)data, strlen(data),
         signature, &signature_len);

    /* Signature encode */
    unsigned char *signature_encoded = (unsigned char *)kore_calloc(((4 * signature_len) / 3 ) + 2, sizeof(unsigned char));
    base64url_encode(signature, signature_len, signature_encoded, signature_encoded_length);

    return signature_encoded;
}

char *
jwt_token_new(const char *subject, const char *audience)
{
    /* Hard coded header */
    unsigned char *header_encoded = (unsigned char *)HEADER_ENCODED;
    unsigned char *payload_encoded = NULL;
    unsigned char *signature_encoded = NULL;
    size_t header_encoded_length = 36;
    size_t payload_encoded_length;
    size_t signature_encoded_length;

    char *rawkey = NULL;
    size_t rawkey_len;

    char *key = app_key();
    kore_base64_decode(key, (u_int8_t **)&rawkey, &rawkey_len);

    /* Generate payload */
    payload_encoded = generate_payload(app_domainname(), subject, audience, &payload_encoded_length);

    /*
     * Prepare signature input
     * Generate signature over data
     */
    char *data = (char *)kore_calloc(header_encoded_length + 1 + payload_encoded_length + 1, sizeof(unsigned char));
    sprintf(data, "%s.%s", header_encoded, payload_encoded);
    signature_encoded = sign_payload(rawkey, rawkey_len, data, &signature_encoded_length);
    kore_free(data);

    /* Concat parts */
    char *token = (char *)kore_calloc(header_encoded_length + 1 + payload_encoded_length + 1 + signature_encoded_length + 1, sizeof(unsigned char));
    sprintf(token, "%s.%s.%s", header_encoded, payload_encoded, signature_encoded);

    kore_free(signature_encoded);
    kore_free(payload_encoded);

    return token;
}

static int
check_signature(char *header, char *payload, char *hash)
{
    unsigned char *signature_encoded = NULL;
    char *rawkey = NULL;
    size_t signature_encoded_length;
    size_t rawkey_len;
    int ret = 0;

    /* Decode key */
    kore_base64_decode(app_key(), (u_int8_t **)&rawkey, &rawkey_len);

    /* Calculate hash over incomming token */
    char *data = (char *)kore_calloc(strlen(header) + 1 + strlen(payload) + 1, sizeof(char));
    sprintf(data, "%s.%s", header, payload);
    signature_encoded = sign_payload(rawkey, rawkey_len, data, &signature_encoded_length);
    kore_free(data);

    /* Compare incomming hash against caculated hash */
    ret = strcmp((const char *)hash, (const char *)signature_encoded) ? 0 : 1;

    kore_free(signature_encoded);
    return ret;
}

int
// jwt_verify(char *token, struct *jwt)
jwt_verify(char *token)
{
    /* Return if no JSON object */
    if (token[0] != 'e' || token[1] != 'y')
        return 0;

    char *parts[4];
    kore_split_string(token, ".", parts, 4);

    /* Sanity check */
    if (parts[0] == NULL || parts[1] == NULL || parts[2] == NULL)
        return 0;

    /* Header must match */
    if (strcmp(parts[0], HEADER_ENCODED))
        return 0;

    /* Bail if signature is not mached */
    if (!check_signature(parts[0], parts[1], parts[2]))
        return 0;



    size_t payload_len;
    unsigned char *payload = (unsigned char *)kore_calloc(((3 * strlen(parts[1])) / 4 ) + 1, sizeof(unsigned char));
    base64url_decode((const unsigned char *)parts[1], strlen(parts[1]), payload, &payload_len);
    payload[payload_len] = '\0';

    const char *path_iat[] = {"iat", NULL};
    const char *path_exp[] = {"exp", NULL};

    yajl_val jwt = yajl_tree_parse((const char *)payload, NULL, 0);
    yajl_val obj_issued = yajl_tree_get(jwt, path_iat, yajl_t_number);
    yajl_val obj_expire = yajl_tree_get(jwt, path_exp, yajl_t_number);

    /* Token not yet active */
    if ((long long int)time(NULL) < YAJL_GET_INTEGER(obj_issued)) {
        yajl_tree_free(jwt);
        kore_free(payload);
        return 0;
    }

    /* Token expired */
    if ((long long int)time(NULL) > YAJL_GET_INTEGER(obj_expire)) {
        yajl_tree_free(jwt);
        kore_free(payload);
        return 0;
    }

    yajl_tree_free(jwt);
    kore_free(payload);
    return 1;
}
