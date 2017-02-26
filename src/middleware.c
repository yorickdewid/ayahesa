/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

/*
|--------------------------------------------------------------------------
| HTTP Middleware
|--------------------------------------------------------------------------
|
| Here is where you can register middleware handlers. Middleware routines
| are called on defined routes and can prevent unauthorized access, rewrite
| data or redirect traffic.
|
*/

#include <ayahesa.h>

middleware(auth_basic)
{
    if (!request->hdlr_extra) {
        request->hdlr_extra = kore_malloc(sizeof(struct request_data));
        memset(request->hdlr_extra, '\0', sizeof(struct request_data));
    }

    /* Protect route with basic authentication */
    if (!http_basic_auth(request, STATUSPAGE_AUTH)) {
        http_response_header(request, "www-authenticate", "Basic realm=\"Baisc auth\"");
        http_response_header(request, "content-type", "text/html");
        http_report(request, 401, "Not Found");

        /* Release session tree */
        struct request_data *session = (struct request_data *)request->hdlr_extra;
        tree_free(session->session);
        kore_free(request->hdlr_extra);
        request->hdlr_extra = NULL;

        return_error();
    }

    /* Find user part */
    char *principal = kore_strdup(STATUSPAGE_AUTH);//TODO: needs a kore_free
    char *split = strchr(principal, ':');
    split[0] = '\0';

    //TODO: Free internals
    struct request_data *auth = (struct request_data *)request->hdlr_extra;
    auth->auth.object_id = 1;
    auth->auth.principal = principal;

    return_ok();
}

middleware(auth_jwt)
{
    struct jwt jwt;

    memset(&jwt, '\0', sizeof(struct jwt));
    if (!request->hdlr_extra)
        request->hdlr_extra = kore_calloc(1, sizeof(struct request_data));

    char *method = strtok(data, " ");
    if (!method)
        return_error();

    /* Require bearer authentication */
    if (!strcmp(strtolower(method), "bearer")) {
        char *token = strtok(NULL, " ");

        /* Verify token */
        if (jwt_verify(token, &jwt)) {

            //TODO: Free internals
            struct request_data *auth = (struct request_data *)request->hdlr_extra;
            auth->auth.object_id = jwt.oid;
            auth->auth.principal = kore_strdup(jwt.sub);

            /* Cleanup */
            kore_free(jwt.iss);
            kore_free(jwt.sub);
            kore_free(jwt.aud);

            return_ok();
        }

        /* Cleanup */
        kore_free(jwt.iss);
        kore_free(jwt.sub);
        kore_free(jwt.aud);
    }

    return_error();
}
