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

#include "assets.h"

int
view(struct http_request *request, const char *view)
{
    struct kore_buf		*buffer;
    size_t              length = 0;

    buffer = kore_buf_alloc(asset_len_test_html);
    kore_buf_append(buffer, asset_test_html, asset_len_test_html);

    /* Easy substitution */
    kore_buf_replace_string(buffer, "@year", "2017", 4);
    kore_buf_replace_string(buffer, "@instance", "SRV1", 4);
    kore_buf_replace_string(buffer, "@date", "Sun Feb  5 00:48:22 CET 2017", 28);
    kore_buf_replace_string(buffer, "@name", "Ayahesa", 7);
    kore_buf_replace_string(buffer, "@user", "Eve", 3);
    kore_buf_replace_string(buffer, "@if(1)", "", 0);
    kore_buf_replace_string(buffer, "@endif", "", 0);

    char *str = kore_buf_stringify(buffer, &length);

    http_response_header(request, "content-type", "text/html");
    http_response(request, 200, str, length);

    kore_buf_cleanup(buffer);

	return (KORE_RESULT_OK);
}
