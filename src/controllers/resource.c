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
#include <quid.h>

#include <fcntl.h>

#include "../core/util.h"
#include "../core/afile.h"

const char *mime_type(const char *ext);

static char *
fetch_file(char *filename, size_t *file_size)
{
    AYAFILE *fp = afopen(filename, "rb");
    if (!fp)
        return NULL;

    /* Set key */
    afkey((unsigned char *)"password", fp);

    /* Determine file size */
    *file_size = afsize(fp);

    unsigned char *content = kore_calloc(1, *file_size);
    afread(content, *file_size, 1, fp);

    afclose(fp);
    return (char *)content;
}

static int
store_file(struct http_file	*file, char *filename)
{
    unsigned char       buf[BUFSIZ];
    ssize_t             ret, written;
    int                 error = 0;

    /* Open file */
    AYAFILE *fp = afopen(filename, "wb");
    if (!fp)
        return -1;

    /* Set key */
    afkey((unsigned char *)"password", fp);

    /* Keep writing to file */
    for (;;) {
        ret = http_file_read(file, buf, sizeof(buf));
        if (ret == -1) {
            kore_log(LOG_ERR, "failed to read from file");
            error = 1;
            break;
        }

        if (ret == 0)
            break;

        written = afwrite(buf, ret, 1, fp);
        if (written < 0) {
            kore_log(LOG_ERR, "write(%s): %s", file->filename, errno_s);
            error = 1;
            break;
        }
    }

    /* Close file */
    if (afclose(fp) < 0)
        kore_log(LOG_WARNING, "fclose(%s): %s", file->filename, errno_s);

    /* Unlink on error */
    if (error) {
        if (unlink(file->filename) < 0) {
            kore_log(LOG_WARNING, "unlink(%s): %s", file->filename, errno_s);
            return -1;
        }
    }

    return 0;
}

static int
get_resource(struct http_request *request, struct request_data *auth)
{
    char        *uuid, *ext;
    char        filename[256];
    size_t      file_size;

    /* Fetch arguments */
    http_populate_get(request);
    if (!http_argument_get_string(request, "id", &uuid) ||
        !http_argument_get_string(request, "ext", &ext)) {
        http_response(request, 412, NULL, 0);
        return_ok();
    }

    /* Resource location */
    snprintf(filename, 256, "%s/app/%d-%s.%s", app_storage(), auth->auth.object_id, uuid, ext);

    char *string = fetch_file(filename, &file_size);
    if (!string) {
        http_response(request, 404, NULL, 0);
        return_ok();
    }

    /* Fire success download event */
    fire(EVENT_DOWNLOAD_SUCCESS, uuid);

    http_response_header(request, "content-type", mime_type(ext));
    http_response(request, 200, string, file_size);

    kore_free(string);
    return_ok();
}

static int
put_resource(struct http_request *request, struct request_data *auth)
{
    struct http_file	    *file;
    char                    filename[256];
    cuuid_t                 quid;
    char                    uuid[QUID_FULLLEN + 1];

    /* Parse the multipart data that was present */
    http_populate_multipart_form(request);

    /* Find our file */
    if ((file = http_file_lookup(request, "file")) == NULL) {
        http_response(request, 400, NULL, 0);
        return_ok();
    }

    /* Generate file identifier */    
    quid_create(&quid, IDF_STRICT, CLS_CMON, NULL);
    quid_tostring(&quid, uuid);
    uuid[QUID_FULLLEN - 1] = '\0';
    memmove(uuid, uuid+1, QUID_FULLLEN);

    /* Resource location */
    snprintf(filename, 256, "%s/app/%d-%s.%s", app_storage(), auth->auth.object_id, uuid, file_extension(file->filename));

    /* Store the file */
    if (store_file(file, filename) < 0) {
        kore_log(LOG_ERR, "failed to read from file");
        http_response(request, 500, NULL, 0);
        return_ok();
    }

    /* Fire success upload event */
    fire(EVENT_UPLOAD_SUCCESS, uuid);

    http_response_header(request, "content-type", "text/plain");
    http_response(request, 200, uuid, QUID_FULLLEN - 2);
    return_ok();
}

/**
 * Resource controller
 *
 */
controller(resource)
{
    /* Must be authenticated */
    if (!request->hdlr_extra)
        return_error();
    struct request_data *auth = (struct request_data *)request->hdlr_extra;
    if (!auth->auth.principal || !auth->auth.object_id)
        return_error();

    /* Determine action */
    switch (request->method) {
        case HTTP_METHOD_GET:
            return get_resource(request, auth);
        case HTTP_METHOD_POST:
            return put_resource(request, auth);
        default:
            break;
    }

    http_response_header(request, "allow", "GET,POST");
    http_response(request, HTTP_STATUS_METHOD_NOT_ALLOWED, NULL, 0);
    return_ok();
}
