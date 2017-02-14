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

#include <openssl/evp.h>
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
    hdr.keysz = 128;

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
afkey(unsigned char key[], AYAFILE *afp)
{
    afp->key = key;
}

size_t
afread(void *ptr, size_t size, size_t nmemb, AYAFILE *afp)
{
    if (!afp->has_header) {
        if (read_header(afp) < 0)
            return 0;
    }

    // if (!afp->key)
        // return 0;

    return fread(ptr, size, nmemb, afp->fp);
}

size_t
afwrite(const void *ptr, size_t size, size_t nmemb, AYAFILE *afp)
{
    // if (!afp->key)
        // return 0;

    if (!afp->has_header)
        write_header(afp);

    return fwrite(ptr, size * nmemb, nmemb, afp->fp);
}

size_t
afsize(AYAFILE *afp)
{
    size_t filesz;
    fseek(afp->fp, 0, SEEK_END);
    filesz = ftell(afp->fp);
    fseek(afp->fp, 0, SEEK_SET);

    if (filesz < 1)
        return 0;

    return filesz - sizeof(struct cryptfile);
}

int
afclose(AYAFILE *afp)
{
    int ret = 0;

    ret = fclose(afp->fp);
    kore_free(afp);
    return ret;
}

void file_crypt(int should_encrypt, FILE *ifp, FILE *ofp, unsigned char *ckey, unsigned char *ivec);

void file_crypt(int should_encrypt, FILE *ifp, FILE *ofp, unsigned char *ckey, unsigned char *ivec) {
    const unsigned int bufsize = 4096;
    unsigned char *read_buf = malloc(bufsize);

    int out_len;
    EVP_CIPHER_CTX ctx;

    EVP_CipherInit(&ctx, EVP_aes_256_cbc(), ckey, ivec, should_encrypt);
    unsigned int blocksize = EVP_CIPHER_CTX_block_size(&ctx);
    unsigned char *cipher_buf = malloc(bufsize + blocksize);

    while (1) {
        unsigned int numRead = fread(read_buf, sizeof(unsigned char), bufsize, ifp);
        EVP_CipherUpdate(&ctx, cipher_buf, &out_len, read_buf, numRead);
        fwrite(cipher_buf, sizeof(unsigned char), out_len, ofp);
        if (numRead < bufsize)
            break;
    }

    // Now cipher the final block and write it out.
    EVP_CipherFinal(&ctx, cipher_buf, &out_len);
    fwrite(cipher_buf, sizeof(unsigned char), out_len, ofp);

    // Free memory
    free(cipher_buf);
    free(read_buf);
}
