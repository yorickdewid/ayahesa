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
    int bytes_read, counter = 0;
    unsigned char *indata = kore_malloc(size * nmemb);

    unsigned char *ckey = (unsigned char *)kore_strdup((const char *)afp->key);
    unsigned char *civ = (unsigned char *)kore_strdup((const char *)afp->iv);

    /* Set the encryption key */
    AES_set_encrypt_key(ckey, 128, &key);

    /* Set pointer to offset 0 */
    rewind(afp->fp);

    while (size) {
        bytes_read = fread(indata, 1, nmemb, afp->fp);

        printf(">> %d\n", bytes_read);

        AES_cfb128_encrypt(indata, ptr, bytes_read, &key, civ, &counter, AES_DECRYPT);
        break;
    }

    // kore_free(ckey);
    // kore_free(civ);

    kore_free(indata);
    return 0;
}

size_t
afwrite(const void *ptr, size_t size, size_t nmemb, AYAFILE *afp)
{
    AES_KEY key;
    int counter = 0;
    unsigned char *outdata = kore_malloc(size * nmemb);

    unsigned char *ckey = (unsigned char *)kore_strdup((const char *)afp->key);
    unsigned char *civ = (unsigned char *)kore_strdup((const char *)afp->iv);

    /* Set the encryption key */
    AES_set_encrypt_key(ckey, 128, &key);

    /* Set pointer to offset 0 */
    rewind(afp->fp);

    while (size) {
        AES_cfb128_encrypt(ptr, outdata, nmemb, &key, civ, &counter, AES_ENCRYPT);

        fwrite(outdata, 1, nmemb, afp->fp);
        if (nmemb < AES_BLOCK_SIZE)
            break;
    }

    // kore_free(p_ckey);
    // kore_free(civ);

    kore_free(outdata);
    return 0;
}

int
afseek(AYAFILE *afp, long offset, int whence)
{
    return fseek(afp->fp, offset, whence);
}

long
aftell(AYAFILE *afp)
{
    return ftell(afp->fp);
}

void
arewind(AYAFILE *afp)
{
    rewind(afp->fp);
}

int
afclose(AYAFILE *afp)
{
    int ret = 0;

    ret = fclose(afp->fp);
    kore_free(afp);
    return ret;
}