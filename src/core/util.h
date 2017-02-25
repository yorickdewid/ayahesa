/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef _AYAHESA_UTIL_H_
#define _AYAHESA_UTIL_H_

char        *generate_instance_id(void);
void        random_string(unsigned char buffer[], size_t, int);
const char  *file_extension(const char *filename);
int         date_year(void);

void        aya_buf_replace_string(struct kore_buf *, char *, size_t,void *, size_t);
void        aya_buf_replace_first_string(struct kore_buf *, char *, void *, size_t);

#endif // _AYAHESA_UTIL_H_
