/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef _AYAHESA_COMPAT_H_
#define _AYAHESA_COMPAT_H_

#include <string.h>

/*
 * Dropin standard library functions
 */

#define trim(s) \
    lskip(rstrip(s))

#define aya_malloc(s) \
    kore_malloc(s)

#define aya_free(p) \
    kore_free(p)

void *aya_calloc(size_t, size_t);
char *aya_strncpy0(char *, const char *, size_t);
size_t aya_strlcpy(char *, const char *, const size_t);
size_t aya_strlcat(char *, const char *, const size_t);
char *aya_itoa(int);
char *strtolower(char *str);
int strisalpha(char *str);
char *rstrip(char* s);
char *lskip(char *s);

#endif // _AYAHESA_COMPAT_H_
