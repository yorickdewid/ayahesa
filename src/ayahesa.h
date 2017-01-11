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

#include "compat.h"
#include "defs.h"

#include <kore/kore.h>
#include <kore/http.h>

#define VERSION "Ayahesa/0.2"
#define CONFIG  "conf/framework.ini"
#define TESTCODE

#ifdef DEBUG
# define TESTCODE
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
char *generate_instance_id(void);
const char *get_cookie(struct http_request *request, const char *name);

void application_create(app_t **app);
void application_config(app_t *app, const char *configfile);
void application_release(app_t *app);

void cache_remove(app_t *app, const char *key);
void cache_put_int(app_t *app, const char *key, int value);
void cache_put_float(app_t *app, const char *key, float value);
void cache_put_str(app_t *app, const char *key, const char *value);
void cache_put_ptr(app_t *app, const char *key, void *value);


#endif // _AYAHESA_H_
