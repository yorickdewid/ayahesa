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
