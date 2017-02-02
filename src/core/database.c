/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include "../include/ayahesa.h"
#include "tree.h"

void database_init(void);

#define variant_register(n) \
static void n##_register(const char *); \
void n##_register(const char *dbname) \
{ \
    char connstring[128]; \
    char *user = NULL; \
    char *password = NULL; \
    char *host = NULL; \
\
    snprintf(connstring, 128, "dbname=%s ", dbname); \
\
    config_get_str(root_app, #n ".user", &user); \
    if (user) { \
        strcat(connstring, "user="); \
        strcat(connstring, user); \
        strcat(connstring, " "); \
    } \
\
    config_get_str(root_app, #n ".password", &password); \
    if (password) { \
        strcat(connstring, "password="); \
        strcat(connstring, password); \
        strcat(connstring, " "); \
    } \
\
    config_get_str(root_app, #n ".host", &host); \
    if (host) { \
        strcat(connstring, "host="); \
        strcat(connstring, host); \
        strcat(connstring, " "); \
    } \
\
    kore_pgsql_register(#n, connstring); \
}

variant_register(db)
variant_register(dbr)
variant_register(dbrw)

void
database_init(void)
{
    char *dbname = NULL;
    char *dbrwname = NULL;
    char *dbrname = NULL;

    /* Common database */
    config_get_str(root_app, "db.name", &dbname);
    if (dbname) {
        db_register(dbname);
        return;
    }

    /* Read/write database */
    config_get_str(root_app, "dbrw.name", &dbrwname);
    if (dbrwname) {
        dbrw_register(dbrwname);
    }

    /* Read database */
    config_get_str(root_app, "dbr.name", &dbrname);
    if (dbrname) {
        dbr_register(dbrname);
    }
}
