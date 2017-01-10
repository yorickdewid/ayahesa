/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include "ayahesa.h"

#include <ctype.h>

char *
generate_instance_id(void)
{
    static const char ar[] = "abcdefghijklmnopqrstuvwxyz";
    unsigned int rnd;

    char *str = (char *)kore_malloc(10);

    str[0] = ar[rand() % strlen(ar)];
    str[1] = ar[rand() % strlen(ar)];
    str[2] = '-';

    int i;
    for (i=3; i<9; ++i) {
        rnd = rand() % strlen(ar);
        str[i] = toupper(ar[rnd]);
    }

    str[9] = '\0';

    return str;
}
