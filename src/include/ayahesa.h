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

#include "core/compat.h"
#include "core/defs.h"

#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/jsonrpc.h>

#define VERSION "Ayahesa/0.4"
#define CONFIG  "conf/framework.ini"

#ifdef DEBUG
# define STATUSPAGE
#endif

#ifdef TESTCASE
# define STATUSPAGE
#endif

#ifdef STATUSPAGE
# define STATUSPAGE_AUTH "eve:ABC@1239"
#endif

#define T_FLAG_READONLY 0x1

/*
 * Application internal structure
 *
 * type:    Datatype
 * flags:   Additional tree options
 * key:     Key-value identifier
 * value:   Data object
 * child:   Child trees
 */
struct app_tree {
    enum {
        T_NULL = 0,
        T_INT,
        T_FLOAT,
        T_STRING,
        T_POINTER,
    } type;
    char flags;
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

/*
 * Prototypes
 */
const char *http_get_cookie(struct http_request *, const char *);
int http_basic_auth(struct http_request *, const char *);
char *http_remote_addr(struct http_request *);

void application_create(app_t **);
void application_config(app_t *, const char *);
void application_release(app_t *);
void application_prelude(void);

char *application_uptime(app_t *);
unsigned int application_request_count(void);
int application_isdebug(app_t *);
char *application_name(app_t *);
char *application_environment(app_t *);

int jrpc_write_string(struct jsonrpc_request *, void *);
int jrpc_write_string_array_params(struct jsonrpc_request *, void *);

char *jwt_token_new(const char *key, const char *issuer, const char *subject, const char *audience);

#endif // _AYAHESA_H_
