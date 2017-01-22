/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include <ayahesa.h>

/**
 * Resource controller
 *
 */
controller(resource)
{
    char		*uuid;

    if (request->method == HTTP_METHOD_GET)
		http_populate_get(request);

    /* Fetch arguments */
    if (!http_argument_get_string(request, "id", &uuid))
	    http_response(request, 404, NULL, 0);

    FILE *fp = fopen("store/100-Kaas-BV.pdf", "r");
    if (!fp) {
        http_response(request, 500, "Shit went wrong", 15);
        return_ok();
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *string = kore_malloc(fsize + 1);
    fread(string, fsize, 1, fp);
    fclose(fp);

    string[fsize] = '\0';

    http_response_header(request, "content-type", "application/pdf");
	http_response(request, 200, string, fsize);

    kore_free(string);

    return_ok();
}
