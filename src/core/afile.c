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
#include "afile.h"

#include <openssl/aes.h>

AYAFILE *
afopen(const char *path, const char *mode)
{
    AYAFILE *afp = kore_malloc(sizeof(AYAFILE));
    afp->key = NULL;
    afp->iv = NULL;

    afp->fp = fopen(path, mode);
    if (!afp->fp) {
        kore_free(afp);
        return NULL;
    }

    return afp;
}

void
afset(unsigned char key[], unsigned char iv[], AYAFILE *afp)
{
    afp->key = key;
    afp->iv = iv;
}

size_t
afread(void *ptr, size_t size, size_t nmemb, AYAFILE *afp)
{
    AES_KEY key;
    unsigned char iv[32];
    int counter = 0, readsz = 0;

    if (!afp->key || !afp->iv)
        return 0;

    unsigned char *ckey = (unsigned char *)kore_strdup((const char *)afp->key);
    memset(iv, '\0', 32);
    strcpy((char *)iv, (const char *)afp->iv);

    /* Set the encryption key */
    AES_set_encrypt_key(ckey, 128, &key);

    /* Set pointer to offset 0 */
    rewind(afp->fp);

    unsigned int i;
    for (i=0; i<nmemb; ++i) {
        unsigned char *indata = kore_malloc(size);
        readsz += fread(indata, size, 1, afp->fp);

        unsigned char *part = ((unsigned char *)ptr) + (i * size);
        AES_cfb128_encrypt(indata, part, size, &key, iv, &counter, AES_DECRYPT);
        kore_free(indata);
    }

    kore_free(ckey);
    
    return readsz;
}

size_t
afwrite(const void *ptr, size_t size, size_t nmemb, AYAFILE *afp)
{
    AES_KEY key;
    unsigned char iv[32];
    int counter = 0, writesz = 0;

    if (!afp->key || !afp->iv)
        return 0;

    unsigned char *ckey = (unsigned char *)kore_strdup((const char *)afp->key);
    memset(iv, '\0', 32);
    strcpy((char *)iv, (const char *)afp->iv);

    /* Set the encryption key */
    AES_set_encrypt_key(ckey, 128, &key);

    /* Set pointer to offset 0 */
    rewind(afp->fp);

    unsigned int i;
    for (i=0; i<nmemb; ++i) {
        unsigned char *outdata = kore_malloc(size);

        const unsigned char *part = ((const unsigned char *)ptr) + (i * size);
        AES_cfb128_encrypt(part, outdata, size, &key, iv, &counter, AES_ENCRYPT);

        writesz += fwrite(outdata, size, 1, afp->fp);
        kore_free(outdata);
    }

    kore_free(ckey);

    return writesz;
}

int
afclose(AYAFILE *afp)
{
    int ret = 0;

    ret = fclose(afp->fp);
    kore_free(afp);
    return ret;
}