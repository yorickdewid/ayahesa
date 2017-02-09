/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef _AYAHESA_AFILE_H_
#define _AYAHESA_AFILE_H_

#include <string.h>

typedef struct {
    FILE *fp;
    const unsigned char *key;
    const unsigned char *iv;
} AYAFILE;

AYAFILE *afopen(const char *path, const char *mode);
void afset(unsigned char key[], unsigned char iv[], AYAFILE *afp);
size_t afread(void *ptr, size_t size, size_t nmemb, AYAFILE *afp);
size_t afwrite(const void *ptr, size_t size, size_t nmemb, AYAFILE *afp);
int afseek(AYAFILE *afp, long offset, int whence);
long aftell(AYAFILE *afp);
void arewind(AYAFILE *afp);
int afclose(AYAFILE *fp);

#endif // _AYAHESA_AFILE_H_
