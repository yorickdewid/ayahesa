/**
 * Copyright (C) 2016 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef _AYAHESA_H_
#define _AYAHESA_H_

#include <kore/kore.h>
#include <kore/http.h>

#define VERSION "Ayahesa/0.2"
#define DEBUG 1
#define CONFIG  "conf/framework.ini"

#define return_ok() return (KORE_RESULT_OK); 

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

/*
 * Application internal structure
 */
struct app_tree {
    enum {
        T_NULL = 0,
        T_INT,
        T_FLOAT,
        T_STRING,
        T_POINTER,
    } type;
    char *key;
    union {
        int i;
        float f;
        char *str;
        void *ptr;
    } value;
    struct {
        unsigned int alloc_cnt;
        struct app_tree **ptr;
    } child;
};

typedef struct app_tree app_t;

/* Core root definition */
extern app_t *root_app;

#define app_instance_id() \
    root_app->value.str

/*
 * Prototypes
 */
char *generate_instance_id(void);

void application_create(app_t **app);
void application_config(app_t *app, const char *configfile);
void application_release(app_t *app);

void cache_remove(app_t *app, const char *key);
void cache_put_str(app_t *app, const char *key, const char *value);
void cache_put_int(app_t *app, const char *key, int value);

#endif // _AYAHESA_H_
