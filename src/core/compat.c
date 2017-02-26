/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

/**
 * Implement POSIX functions as we obey ISO C
 */

#include "compat.h"

#include <kore/kore.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void *aya_calloc(size_t n, size_t sz) {
    void *ptr = kore_calloc(n, sz);
    if (!ptr)
        return NULL;
    memset(ptr, '\0', (n * sz));
    return ptr;
}

/* 
 * Version of strncpy that ensures dest (size bytes)
 * is null-terminated.
 */
char *
aya_strncpy0(char *dst, const char *src, size_t len)
{
    strncpy(dst, src, len);
    dst[len - 1] = '\0';
    return dst;
}

size_t
aya_strlcpy(char *dst, const char *src, const size_t len)
{
    char        *d = dst;
    const char  *s = src;
    const char  *end = dst + len - 1;

    if (len == 0)
        return 0;

    while ((*d = *s) != '\0') {
        if (d == end) {
            *d = '\0';
            break;
        }

        d++;
        s++;
    }

    while (*s != '\0')
        s++;

    return s - src;
}

size_t
aya_strlcat(char *dst, const char *src, const size_t len)
{
    char		*d = dst;
    const char  *s = src;
    size_t      n = len;
    size_t      dlen;

    if (len == 0)
        return 0;

    while (n-- != 0 && *d != '\0')
        d++;

    dlen = d - dst;
    n = len - dlen;

    if (n == 0)
        return dlen + strlen(s);

    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }

        s++;
    }
    *d = '\0';

    return dlen + (s - src);
}

/* Convert string to lower */
char *
strtolower(char *str)
{
    int i;

    for (i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }

    return str;
}

/* Check if string is of alphabetical contents */
int
strisalpha(char *str)
{
    int i;

    for (i = 0; str[i]; i++) {
        if (!isalpha(str[i]))
            return 0;
    }

    return 1;
}

char *
aya_itoa(int i)
{
    static char buf[19 + 2];
    char *p = buf + 19 + 1;	/* points to terminating '\0' */

    if (i >= 0) {
        do {
        *--p = '0' + (i % 10);
        i /= 10;
        } while (i != 0);
        return p;
    } else {
        do {
            *--p = '0' - (i % 10);
            i /= 10;
        } while (i != 0);
        *--p = '-';
    }
    return p;
}

/* Strip whitespace chars off end of given string, in place. Return s. */
char *
rstrip(char* s)
{
    char *p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
char *
lskip(char *s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}
