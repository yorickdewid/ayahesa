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

#ifdef NEED_STRDUP

char *ayahesa_strdup(const char *src);

#endif // NEED_STRDUP

char *aya_strncpy0(char *, const char *, size_t);
size_t aya_strlcpy(char *, const char *, const size_t);
size_t aya_strlcat(char *, const char *, const size_t);
char *strtolower(char *str);
int strisalpha(char *str);
char *rstrip(char* s);
char *lskip(char *s);

#endif // _AYAHESA_COMPAT_H_
