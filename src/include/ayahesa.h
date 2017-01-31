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

/* Event types */
typedef enum {
    EVENT_AUTH_FAILED = 10,
    EVENT_AUTH_SUCCESS,
} event_type_t;

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

struct jwt {
    char *iss;          /* issuer */
    char *sub;          /* Subject */
    char *aud;          /* Audience */
    long long int oid;  /* Object ID */
    long long int iat;  /* Issued at */
    long long int exp;  /* Expired at */
};

struct aya_trigger {
    event_type_t event;
    void (*cb)(void *);
};

typedef struct app_tree app_t;

/* Core root definition */
extern app_t *root_app;

/*
 * Application operations
 */
void            app_log(const char *);
char *          app_instance(void);
char *          app_uptime(void);
unsigned int    app_request_count(void);
unsigned int    app_active_conncount(void);
int             app_isdebug(void);
char *          app_name(void);
char *          app_environment(void);
int             app_session_lifetime(void);
const char *    app_domainname(void);
char *          app_key(void);

/*
 * HTTP helpers
 */
const char *    http_get_cookie(struct http_request *, const char *);
int             http_basic_auth(struct http_request *, const char *);
char *          http_remote_addr(struct http_request *);
char *          http_report(int code, char *title, size_t *length);

/*
 * Events
 */
void fire(event_type_t type, void *data);

/*
 * JRPC helpers
 */
int jrpc_write_string(struct jsonrpc_request *, void *);

/*
 * JWT operations
 */
char *jwt_token_new(struct jwt *jwt);
int jwt_verify(char *, struct jwt *);

#endif // _AYAHESA_H_
