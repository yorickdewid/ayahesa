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

int		init(int);
int		about(struct http_request *);
int		error(struct http_request *);
int		status(struct http_request *);
int		teapot(struct http_request *);

app_t *root_app = NULL;

int
init(int state)
{
	switch (state) {
		case KORE_MODULE_LOAD:
			kore_log(LOG_NOTICE, "load ayahesa");
			application_create(&root_app);
			application_config(root_app, CONFIG);
			application_release(root_app);//TODO: FOR NOW!
			break;
		case KORE_MODULE_UNLOAD:
			kore_log(LOG_NOTICE, "unload ayahesa");
			application_release(root_app);
			break;
		default:
			kore_log(LOG_NOTICE, "state %d unknown", state);
			break;
	}

	return (KORE_RESULT_OK);
}

#if defined(TESTCODE)

int
about(struct http_request *req)
{
	char buffer[256];
	const char *default_page =
		"<html>"
		"<head><title>Ayahesa</title></head>"
		"<body>"
		"<h2>Ayahesa Core</h2>"
		"<div>Instance %s</div>"
		"<div>" VERSION "</div>"
		"</body>"
		"</html>";

	sprintf(buffer, default_page, root_app->value.str);

	if (req->method != HTTP_METHOD_GET) {
		http_response_header(req, "allow", "get");
		http_response(req, 405, NULL, 0);
		return (KORE_RESULT_OK);
	}

	http_response_header(req, "content-type", "text/html");
	http_response_header(req, "set-cookie", "_umaysess=3745693");
	http_response(req, 200, buffer, strlen(buffer));
	return (KORE_RESULT_OK);
}

int
error(struct http_request *req)
{
	http_response(req, 500, NULL, 0);
	return (KORE_RESULT_OK);
}

int
status(struct http_request *req)
{
	http_response_header(req, "content-type", "text/plain");
	http_response(req, 200, "Server is OK", 12);
	return (KORE_RESULT_OK);
}

int
teapot(struct http_request *req)
{
	http_response_header(req, "content-type", "text/plain");
	http_response(req, 418, "I'm a teapot.", 13);
	return (KORE_RESULT_OK);
}

#endif // DEBUG
