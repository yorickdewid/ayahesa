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

extern struct aya_trigger trigger[];

void
fire(event_type_t type, void *data)
{
    int i;
    for (i = 0; trigger[i].cb != NULL; ++i) {
        if (trigger[i].event == type)
            trigger[i].cb(data);
    }
}
