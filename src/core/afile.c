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
#include "util.h"
#include "afile.h"

#include <openssl/aes.h>

#define FILE_MAGIC 0xfe8a

struct cryptfile {
    uint16_t magic;
    uint16_t keysz;
    uint8_t iv[32];
};

static int
read_header(AYAFILE *afp)
{
    static struct cryptfile hdr;

    /* Set pointer to offset 0 */
    rewind(afp->fp);

    memset(&hdr, '\0', sizeof(struct cryptfile));
    fread(&hdr, sizeof(struct cryptfile), 1, afp->fp);
    if (hdr.magic != FILE_MAGIC)
        return -1;

    afp->iv = hdr.iv;
    afp->keysz = hdr.keysz;
    afp->has_header = 1;

    return 0;
}

static void
write_header(AYAFILE *afp)
{
    static struct cryptfile hdr;

    memset(&hdr, '\0', sizeof(struct cryptfile));
    random_string(hdr.iv, 16, 1);

    hdr.magic = FILE_MAGIC;
    hdr.keysz = 192;

    /* Set pointer to offset 0 */
    rewind(afp->fp);

    fwrite(&hdr, sizeof(struct cryptfile), 1, afp->fp);

    afp->iv = hdr.iv;
    afp->keysz = hdr.keysz;
    afp->has_header = 1;
}

AYAFILE *
afopen(const char *path, const char *mode)
{
    AYAFILE *afp = kore_malloc(sizeof(AYAFILE));
    afp->key = NULL;
    afp->iv = NULL;
    afp->has_header = 0;

    afp->fp = fopen(path, mode);
    if (!afp->fp) {
        kore_free(afp);
        return NULL;
    }

    return afp;
}

void
afset(unsigned char key[], AYAFILE *afp)
{
    afp->key = key;
}

size_t
afread(void *ptr, size_t size, size_t nmemb, AYAFILE *afp)
{
    AES_KEY key;
    unsigned char iv[32];
    int counter = 0, readsz = 0;

    if (!afp->has_header)
        if (read_header(afp) < 0)
            return 0;

    if (!afp->key)
        return 0;

    unsigned char *ckey = (unsigned char *)kore_strdup((const char *)afp->key);
    memset(iv, '\0', 32);
    strcpy((char *)iv, (const char *)afp->iv);

    /* Set the encryption key */
    AES_set_encrypt_key(ckey, afp->keysz, &key);

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

    if (!afp->key)
        return 0;

    if (!afp->has_header)
        write_header(afp);

    unsigned char *ckey = (unsigned char *)kore_strdup((const char *)afp->key);
    memset(iv, '\0', 32);
    strcpy((char *)iv, (const char *)afp->iv);

    /* Set the encryption key */
    AES_set_encrypt_key(ckey, afp->keysz, &key);

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