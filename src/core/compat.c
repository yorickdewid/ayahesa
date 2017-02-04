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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
char *
strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
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
