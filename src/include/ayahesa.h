/**
 * Copyright (C) 2017 Quenza Inc.
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

#define VERSION "Ayahesa/0.6"
#define CONFIG  "conf/framework.ini"

#ifdef DEBUG
# define STATUSPAGE
# define OPT_ROUTES
#endif

#ifdef TESTCASE
# define STATUSPAGE
# define OPT_ROUTES
#endif

#ifdef STATUSPAGE
# define STATUSPAGE_AUTH "eve:ABC@123"
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

struct request_data {
    struct {
        unsigned int object_id;
        char *principal;
    } auth;
};

typedef struct app_tree app_t;

/* Core root definition */
extern app_t *root_app;

void application_create(app_t **);
void application_config(app_t *, const char *);
void application_release(app_t *);
void application_prelude(struct connection *);
void application_postproc(struct connection *c);

/*
 * Application operations
 */
char *          application_uptime(app_t *);
unsigned int    application_request_count(void);
unsigned int    application_active_conncount(void);
int             application_isdebug(app_t *);
char *          application_name(app_t *);
char *          application_environment(app_t *);
int             application_session_lifetime(app_t *);
const char *    application_domainname(app_t *app);
char *          application_key(app_t *app);

/*
 * HTTP helpers
 */
const char *    http_get_cookie(struct http_request *, const char *);
int             http_basic_auth(struct http_request *, const char *);
char *          http_remote_addr(struct http_request *);
char *          http_report(int code, char *title, size_t *length);

/*
 * JRPC helpers
 */
int jrpc_write_string(struct jsonrpc_request *, void *);

/*
 * JWT operations
 */
char *jwt_token_new(const char *subject, const char *audience);
int jwt_verify(char *token);

#endif // _AYAHESA_H_
