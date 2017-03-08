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
#include "core/tree.h"

#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/jsonrpc.h>
#include <kore/pgsql.h>

#include "assets.h"

#define AYA_VERSION         "0.6.4"
#define AYA_VERSION_STRING  "Ayahesa/" AYA_VERSION
#define CONFIG              "conf/framework.ini"

#ifdef DEBUG
# define OPT_ROUTES
#endif

#ifdef TESTCASE
# define OPT_ROUTES
#endif

#define T_FLAG_READONLY 0x1

/* Event types */
typedef enum {
    EVENT_AUTH_FAILED = 10,
    EVENT_AUTH_SUCCESS,
    EVENT_DOWNLOAD_SUCCESS,
    EVENT_UPLOAD_SUCCESS,
} event_type_t;

struct request_data {
    struct {
        unsigned int object_id;
        char *principal;
    } auth;
    struct app_tree *session;
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

struct aya_providers {
    char module[64];
    void (*cb_load)(void);
    void (*cb_unload)(void);
};

typedef struct app_tree app_t;

/* Core root definition */
extern app_t *root_app;

/*
 * Application operations
 */
void            app_log(const char *, ...);
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
char *          app_storage(void);
char *          app_basic_auth(void);

/*
 * HTTP helpers
 */
const char *    http_get_cookie(struct http_request *, const char *);
int             http_basic_auth(struct http_request *, const char *);
char *          http_remote_addr(struct http_request *);
char *          http_auth_principal(struct http_request *);
void            http_report(struct http_request *, int, char *);

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
int   jwt_verify(char *, struct jwt *);

/*
 * Crypt operations
 */
char *crypt_password_hash(const char *);
int   crypt_password_verify(const char *, char *);
void  crypt_encrypt(const unsigned char *, unsigned char *, unsigned char *);
void  crypt_decrypt(const unsigned char *, unsigned char *, unsigned char *);
void  crypt_sign(const unsigned char *, size_t, unsigned char *, size_t *,
        const unsigned char *, size_t);

int   http_view(struct http_request *, int, const char *);

#endif // _AYAHESA_H_
