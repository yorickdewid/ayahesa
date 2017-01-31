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

static char *
fetch_file(char *file, size_t *file_size)
{
    FILE *fp = fopen(file, "r");
    if (!fp)
        return NULL;

    /* Determine file size */
    fseek(fp, 0, SEEK_END);
    *file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    //TODO: mmap large files
    char *string = kore_malloc(*file_size + 1);
    fread(string, *file_size, 1, fp);
    fclose(fp);

    string[*file_size] = '\0';
    return string;
}

/**
 * Resource controller
 *
 */
controller(resource)
{
    char		*uuid, *ext;
    char        filename[255];
    size_t      file_size;

    /* Must be authenticated */
    if (!request->hdlr_extra)
        return_error();
    struct request_data *auth = (struct request_data *)request->hdlr_extra;
    if (!auth->auth.principal || !auth->auth.object_id)
        return_error();

    /* Call is a HTTP GET request for sure */
	http_populate_get(request);

    /* Fetch arguments */
    if (!http_argument_get_string(request, "id", &uuid) ||
        !http_argument_get_string(request, "ext", &ext)) {
	    http_response(request, 404, NULL, 0);
        return_ok();
    }

    snprintf(filename, 255, "%s/app/%d-%s.%s", app_storage(), auth->auth.object_id, uuid, ext);

    char *string = fetch_file(filename, &file_size);
    if (!string) {
        http_response(request, 404, NULL, 0);
        return_ok();
    }

    http_response_header(request, "content-type", "image/jepg");
	http_response(request, 200, string, file_size);

    kore_free(string);

    return_ok();
}
