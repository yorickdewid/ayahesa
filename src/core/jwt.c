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

#include <time.h>

#include "base64url.h"

#define HEADER_ENCODED  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"

static unsigned char *
generate_payload(struct jwt *jwt, size_t *payload_encoded_length)
{
    cuuid_t jti;
    yajl_gen tree = yajl_gen_alloc(NULL);
    const char *issuer = app_domainname();
    long long int issued_at = time(NULL);
    long long int expire_at = issued_at + app_session_lifetime();
    char jti_str[QUID_FULLLEN + 1];
    
    /* Generate JTI */    
    quid_create(&jti, IDF_SIGNED | IDF_TAGGED, CLS_CMON, "JWT");
    quid_tostring(&jti, jti_str);

    yajl_gen_map_open(tree);
    yajl_gen_string(tree, (const unsigned char *)"iss", 3);
    yajl_gen_string(tree, (const unsigned char *)issuer, strlen(issuer));
    yajl_gen_string(tree, (const unsigned char *)"sub", 3);
    yajl_gen_string(tree, (const unsigned char *)jwt->sub, strlen(jwt->sub));
    yajl_gen_string(tree, (const unsigned char *)"oid", 3);
    yajl_gen_integer(tree, jwt->oid);
    yajl_gen_string(tree, (const unsigned char *)"aud", 3);
    yajl_gen_string(tree, (const unsigned char *)jwt->aud, strlen(jwt->aud));
    yajl_gen_string(tree, (const unsigned char *)"iat", 3);
    yajl_gen_integer(tree, issued_at);
    yajl_gen_string(tree, (const unsigned char *)"exp", 3);
    yajl_gen_integer(tree, expire_at);
    yajl_gen_string(tree, (const unsigned char *)"jti", 3);
    yajl_gen_string(tree, (const unsigned char *)jti_str, QUID_FULLLEN);
    yajl_gen_map_close(tree);

    /* Object to string */
    const unsigned char *payload;
    size_t payload_len;
    yajl_gen_get_buf(tree, &payload, &payload_len);
    
    /* String encode */
    unsigned char *payload_encoded = (unsigned char *)kore_calloc(((4 * payload_len) / 3 ) + 2, sizeof(unsigned char));
    base64url_encode(payload, payload_len, payload_encoded, payload_encoded_length);

    yajl_gen_clear(tree);
    yajl_gen_free(tree);

    /* Complete the JWT */
    jwt->iat = issued_at;
    jwt->exp = expire_at;

    return payload_encoded;
}

static unsigned char *
sign_payload(char *key, size_t key_length, const char *data, size_t *signature_encoded_length)
{
    size_t signature_len = 0;
    unsigned char signature[36];

    crypt_sign((const unsigned char *)data, strlen(data),
        signature, &signature_len,
        (const unsigned char *)key, key_length);

    /* Signature encode */
    unsigned char *signature_encoded = (unsigned char *)kore_calloc(((4 * signature_len) / 3 ) + 2, sizeof(unsigned char));
    base64url_encode(signature, signature_len, signature_encoded, signature_encoded_length);

    return signature_encoded;
}

char *
jwt_token_new(struct jwt *jwt)
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
    payload_encoded = generate_payload(jwt, &payload_encoded_length);

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

static void
parse_payload(char *payload, struct jwt *jwt)
{
    size_t payload_decoded_length;
    const char *path_iss[] = {"iss", NULL};
    const char *path_sub[] = {"sub", NULL};
    const char *path_aud[] = {"aud", NULL};
    const char *path_oid[] = {"oid", NULL};
    const char *path_iat[] = {"iat", NULL};
    const char *path_exp[] = {"exp", NULL};

    unsigned char *payload_decoded = (unsigned char *)kore_calloc(((3 * strlen(payload)) / 4 ) + 1, sizeof(unsigned char));
    base64url_decode((const unsigned char *)payload, strlen(payload), payload_decoded, &payload_decoded_length);
    payload_decoded[payload_decoded_length] = '\0';

    yajl_val tree = yajl_tree_parse((const char *)payload_decoded, NULL, 0);
    yajl_val obj_issuer = yajl_tree_get(tree, path_iss, yajl_t_string);
    yajl_val obj_subject = yajl_tree_get(tree, path_sub, yajl_t_string);
    yajl_val obj_audience = yajl_tree_get(tree, path_aud, yajl_t_string);
    yajl_val obj_object = yajl_tree_get(tree, path_oid, yajl_t_number);
    yajl_val obj_issued = yajl_tree_get(tree, path_iat, yajl_t_number);
    yajl_val obj_expire = yajl_tree_get(tree, path_exp, yajl_t_number);

    jwt->iss = kore_strdup(YAJL_GET_STRING(obj_issuer));
    jwt->sub = kore_strdup(YAJL_GET_STRING(obj_subject));
    jwt->aud = kore_strdup(YAJL_GET_STRING(obj_audience));
    jwt->oid = YAJL_GET_INTEGER(obj_object);
    jwt->iat = YAJL_GET_INTEGER(obj_issued);
    jwt->exp = YAJL_GET_INTEGER(obj_expire);

    yajl_tree_free(tree);
    kore_free(payload_decoded);
}

int
jwt_verify(char *token, struct jwt *jwt)
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

    /* Parse token */
    parse_payload(parts[1], jwt);

    /* Token not yet active */
    if ((long long int)time(NULL) < jwt->iat)
        return 0;

    /* Token expired */
    if ((long long int)time(NULL) > jwt->exp)
        return 0;

    return 1;
}
