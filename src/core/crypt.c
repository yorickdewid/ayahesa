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
#include "../include/argon2.h"
#include "../core/util.h"

#include <openssl/x509.h>
#include <openssl/hmac.h>
#include <openssl/aes.h>

#define T_COST  2
#define M_COST  2^10
#define P_COST  1

char *
crypt_password_hash(const char *secret)
{
    unsigned char salt[16];
    char *encoded;
    size_t encodedlen;

    /* Generate salt */
    random_string(salt, 16, 1);

    /* Calculate length */
    encodedlen = argon2_encodedlen(T_COST, M_COST, P_COST, 16, 32, Argon2_i);
    encoded = (char *)kore_malloc(encodedlen);

    /* Calculate hash */
    if (argon2i_hash_encoded(T_COST, M_COST, P_COST, secret, strlen(secret),
                            salt, 16, 32, encoded, encodedlen) != ARGON2_OK) {
        return NULL;
    }

    return encoded;
}

int
crypt_password_verify(const char *encoded, char *secret)
{
    if (argon2i_verify(encoded, secret, strlen(secret)) != ARGON2_OK)
        return 0;

    return 1;
}

void
crypt_sign(const unsigned char *data, size_t datalen,
    unsigned char *signature, size_t *signaturelen,
    const unsigned char *key, size_t keylen)
{
    HMAC(EVP_sha256(),
         key, keylen,
         data, datalen,
         signature, (unsigned int *)signaturelen);
}

void
crypt_encrypt(const unsigned char *data, unsigned char *out, unsigned char *key)
{
    AES_KEY enc_key;

    AES_set_encrypt_key(key, 256, &enc_key);
    AES_encrypt(data, out, &enc_key);      
}

void
crypt_decrypt(const unsigned char *data, unsigned char *out, unsigned char *key)
{
    AES_KEY dec_key;

    AES_set_decrypt_key(key, 256, &dec_key);
    AES_decrypt(data, out, &dec_key);
}
