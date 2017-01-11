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

#define return_ok() \
	return (KORE_RESULT_OK); 

#define http_get() \
	if (request->method != HTTP_METHOD_GET) { \
		http_response_header(request, "allow", "GET"); \
		http_response(request, HTTP_STATUS_METHOD_NOT_ALLOWED, NULL, 0); \
		return (KORE_RESULT_OK); \
	}

#define http_post() \
	if (request->method != HTTP_METHOD_POST) { \
		http_response_header(request, "allow", "POST"); \
		http_response(request, HTTP_STATUS_METHOD_NOT_ALLOWED, NULL, 0); \
		return (KORE_RESULT_OK); \
	}

#define http_post_put() \
	if (request->method != HTTP_METHOD_POST && \
        request->method != HTTP_METHOD_PUT) { \
		http_response_header(request, "allow", "POST,PUT"); \
		http_response(request, HTTP_STATUS_METHOD_NOT_ALLOWED, NULL, 0); \
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
 * Endpoint directives
 */

#define jrpc_invoke(m,f) \
	if (!strcmp(request.method, m)) { \
		extern int jrpc_method_##f(struct jsonrpc_request *request); \
		return jrpc_method_##f(&request); \
	}

#define endpoint(e) \
	int endpoint_##e(struct http_request *); \
    int endpoint_##e(struct http_request *_request) \

#define jrpc_parse() \
	struct jsonrpc_request request; int ret; \
	if (_request->method != HTTP_METHOD_POST && _request->method != HTTP_METHOD_PUT) { \
		http_response_header(_request, "allow", "POST,PUT"); \
		http_response(_request, HTTP_STATUS_METHOD_NOT_ALLOWED, NULL, 0); \
		return (KORE_RESULT_OK); \
	} \
	if ((ret = jsonrpc_read_request(_request, &request)) != 0) { \
		return jsonrpc_error(&request, ret, NULL); \
	}

#define jrpc_return_error() \
	return jsonrpc_error(&request, JSONRPC_METHOD_NOT_FOUND, NULL);

#define jrpc_method(m) \
    int jrpc_method_##m(struct jsonrpc_request *); \
    int jrpc_method_##m(struct jsonrpc_request *request)

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
