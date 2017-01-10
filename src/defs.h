/**
 * Copyright (C) 2016 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef _AYAHESA_DEFS_H_
#define _AYAHESA_DEFS_H_

/*
 * Generic
 */

#define app_instance_id() \
    root_app->value.str

#define return_ok() return (KORE_RESULT_OK); 

#define http_get() \
	if (request->method != HTTP_METHOD_GET) { \
		http_response_header(request, "allow", "get"); \
		http_response(request, 405, NULL, 0); \
		return (KORE_RESULT_OK); \
	}

#define http_post() \
	if (request->method != HTTP_METHOD_POST) { \
		http_response_header(request, "allow", "post"); \
		http_response(request, 405, NULL, 0); \
		return (KORE_RESULT_OK); \
	}

/*
 * HTTP directives
 */

#define route(r) \
    int app_##r(struct http_request *); \
    int app_##r(struct http_request *request)

#define redirect(l) \
    http_response_header(request, "location", l); \
	http_response(request, 302, NULL, 0); \
    return (KORE_RESULT_OK);

#define write_plain(t) \
	http_response_header(request, "content-type", "text/plain"); \
	http_response(request, 200, t, strlen(t)); \
	return (KORE_RESULT_OK);

#define write_html(t) \
	http_response_header(request, "content-type", "text/html"); \
	http_response(request, 200, t, strlen(t)); \
	return (KORE_RESULT_OK);

/*
 * Controller definitions
 */

#define invoke(c) \
    extern int controller_##c(struct http_request *request, void *data); \
    controller_##c(request, NULL);

#define invoke_data(c,d) \
    extern int controller_##c(struct http_request *request, void *data); \
    controller_##c(request, d);

#define controller(c) \
    int controller_##c(struct http_request *, void *); \
    int controller_##c(struct http_request *request, void *data)

#endif // _AYAHESA_DEFS_H_
