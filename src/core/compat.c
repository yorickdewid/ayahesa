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

#ifdef NEED_STRDUP

char *
ayahesa_strdup(const char *src) {
    char *dest = (char *)malloc(strlen(src) + 1);
    if (dest == NULL)
        return NULL;

    strcpy(dest, src);
    return dest;
}

#endif // NEED_STRDUP

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
char *
strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

