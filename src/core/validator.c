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

int     validator_required(struct http_request *, char *);
int     validator_file_extension(struct http_request *, char *);

/*
 * Validator: required
 *
 * - Data cannot be empty
 */
int validator_required(struct http_request *request, char *data)
{
    if (data == NULL)
        return_error();

    if (!strlen(data))
        return_error();

    return_ok();
}

/*
 * Validator: file extension
 *
 * - Data size cannot succeed 8 chars
 * - Data must be alphabetic
 */
int validator_file_extension(struct http_request *request, char *data)
{
    if (data == NULL)
        return_error();

    size_t datalen = strlen(data);
    if (datalen == 0 || datalen > 8)
        return_error();

    if (!strisalpha(data))
        return_error();

    return_ok();
}
