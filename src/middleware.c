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
	if (!request->hdlr_extra)
        request->hdlr_extra = kore_calloc(1, sizeof(struct request_data));

	/* Protect route with basic authentication */
	if (!http_basic_auth(request, STATUSPAGE_AUTH)) {
		size_t len;
		char *report = http_report(401, "Authorization required", &len);

		http_response_header(request, "www-authenticate", "Basic realm=\"Baisc auth\"");
		http_response_header(request, "content-type", "text/html");
		http_response(request, 401, report, len);
		kore_free(report);
		return_ok();
	}

	/* Find user part */
	char *principal = kore_strdup(STATUSPAGE_AUTH);
	char *split = strchr(principal, ':');
	split[0] = '\0';

	//TODO: needs free somewhere
	struct request_data *auth = (struct request_data *)request->hdlr_extra;
	auth->auth.object_id = 1;
	auth->auth.principal = principal;
	// kore_free(principal);

    return_ok();
}

middleware(auth_jwt)
{
	if (!request->hdlr_extra)
        request->hdlr_extra = kore_calloc(1, sizeof(struct request_data));

    char *method = strtok(data, " ");
    if (!method)
        return_error();

	/* Require bearer authentication */
    if (!strcmp(strtolower(method), "bearer")) {
        char *token = strtok(NULL, " ");

		/* Verify token */
		if (jwt_verify(token)) {
            struct request_data *auth = (struct request_data *)request->hdlr_extra;
			auth->auth.object_id = 10017;
			auth->auth.principal = kore_strdup("woei");
			return_ok();
		}
    }

	return_error();
}

