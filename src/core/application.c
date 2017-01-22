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
#include "util.h"
#include "tree.h"
#include "ini.h"

#include <assert.h>
#include <time.h>

struct {
	time_t boot;
	unsigned int req_count;
} internal_counter;

/*
 * Bootstrap the application
 */
void
application_create(app_t **app)
{
    assert(*app == NULL);

    /* Seed random pool */
    srand(time(NULL));

    /* Reset counters */
    internal_counter.boot = time(NULL);
    internal_counter.req_count = 0;

    /* Setup application root tree */
    app_t *root = (app_t *)kore_malloc(sizeof(app_t));
    root->value.str = generate_instance_id();
    root->type = T_STRING;
    root->flags = 0;
    root->key = NULL;
    tree_new_root(root);

    /* Config tree */
    root->child.ptr[TREE_CONFIG] = (app_t *)kore_malloc(sizeof(app_t));
    root->child.ptr[TREE_CONFIG]->flags = T_FLAG_READONLY;
    root->child.ptr[TREE_CONFIG]->type = T_NULL;
    root->child.ptr[TREE_CONFIG]->key = NULL;
    tree_new_root(root->child.ptr[TREE_CONFIG]);

    /* Cache tree */
    root->child.ptr[TREE_CACHE] = (app_t *)kore_malloc(sizeof(app_t));
    root->child.ptr[TREE_CACHE]->flags = 0;
    root->child.ptr[TREE_CACHE]->type = T_NULL;
    root->child.ptr[TREE_CACHE]->key = NULL;
    tree_new_root(root->child.ptr[TREE_CACHE]);

    kore_log(LOG_NOTICE, "application instance %s", root->value.str);

    *app = root; 
}

/*
 * Prepare for new connection
 */
void
application_prelude(struct connection *c)
{
    internal_counter.req_count++;
}

/*
 * Parse INI file and load key,value into config tree
 */
static int
load_config_tree(void *user, const char *section, const char *name, const char *value, int lineno)
{
    app_t *app = (app_t *)user;

    /* Config item must have string value */
    if (!strcmp(section, "app")) {
        if (!strlen(value)) {
            printf("Config item in '[app]' cannot be empty (line:%d)\n", lineno);
            abort();
        }
    }

    /* Redefine debug */
    if (!strcmp(name, "debug")) {
        if (!strcmp(value, "true") || !strcmp(value, "1")) {
            config_put_int(app, "debug", 1);
            kore_log(LOG_NOTICE, "application in debug mode");
        } else {
            config_put_int(app, "debug", 0);
        }
        return 1;
    }

    /* Redefine session time */
    if (!strcmp(section, "app") && !strcmp(name, "session")) {
        int err;
        int time = kore_strtonum(value, 10, 60, 86400, &err);
        config_put_int(app, "app.session", time);
        return 1;
    }

    /* Set instance name */
    if (!strcmp(name, "instance")) {
        kore_free(root_app->value.str);
        root_app->value.str = kore_strdup(value);
        kore_log(LOG_NOTICE, "application instance %s", root_app->value.str);
        return 1;
    }

    /* Prefix section */
    size_t szc = strlen(section);
    if (szc) {
        char *key = (char *)kore_malloc(szc + 1 + strlen(name) + 1);
        strcpy(key, section);
        strcat(key, ".");
        strcat(key, name);

        config_put_str(app, key, value);
        kore_free(key);
        return 1;
    }

    /* Append to tree */
    config_put_str(app, name, value);
    return 1;
}

static void
validate_config(app_t *app)
{
    int has_name = tree_contains(app->child.ptr[TREE_CONFIG], "app.name");
    int has_env = tree_contains(app->child.ptr[TREE_CONFIG], "app.env");
    int has_key = tree_contains(app->child.ptr[TREE_CONFIG], "app.key");
    int has_debug = tree_contains(app->child.ptr[TREE_CONFIG], "debug");

    /* No app_name */
    if (!has_name) {
        kore_log(LOG_ERR, "'app.name' must be set");
        abort();
    }

    /* No app_env */
    if (!has_env) {
        kore_log(LOG_ERR, "'app.env' must be set");
        abort();
    }

    /* No app_key */
    if (!has_key) {
        kore_log(LOG_ERR, "'app.key' must be set");
        abort();
    }

    /* Set debug to false if not found */
    if (!has_debug)
        config_put_int(app, "debug", 0);
}

/*
 * Parse application configuration
 */
void
application_config(app_t *app, const char *configfile)
{
    assert(app != NULL);

    if (ini_parse(configfile, load_config_tree, (void *)app) < 0) {
        kore_log(LOG_ERR, "framework configuration error %s", configfile);
        abort();
    }

    /* Check and default config */
    validate_config(app);

    //TODO: remove
    // tree_dump(app->child.ptr[TREE_CONFIG]);
    // tree_dump(app->child.ptr[TREE_CACHE]);
}

/*
 * Application uptime
 */
char *
application_uptime(app_t *app)
{
    int diff_tm;
    time_t end_tm = 0;
    static char uptime[32];

    time(&end_tm);
    diff_tm = difftime(end_tm, internal_counter.boot);

    int days = diff_tm / 86400;
    int remainder = diff_tm % 86400;
    int hours = remainder / 3600;
    remainder = diff_tm % 3600;
    int minutes = remainder / 60;
    int seconds = remainder % 60;

    sprintf(uptime, "%d days, %02d:%02d:%02d\n", days, hours, minutes, seconds);
    return uptime;
}

/*
 * Application request counter
 */
unsigned int
application_request_count(void)
{
    return internal_counter.req_count;
}

/*
 * Application debug mode check
 */
int
application_isdebug(app_t *app)
{
    int debug = 0;
    tree_get_int(app->child.ptr[TREE_CONFIG], "debug", &debug);

    return debug;
}

/*
 * Application name
 */
char *
application_name(app_t *app)
{
    char *name = NULL;
    tree_get_str(app->child.ptr[TREE_CONFIG], "app.name", &name);

    return name;
}

/*
 * Application environment
 */
char *
application_environment(app_t *app)
{
    char *name = NULL;
    tree_get_str(app->child.ptr[TREE_CONFIG], "app.env", &name);

    return name;
}

/*
 * Application session lifetime in seconds
 */
int
application_session_lifetime(app_t *app)
{
    int session_lifetime = 0;
    tree_get_int(app->child.ptr[TREE_CONFIG], "app.session", &session_lifetime);

    return session_lifetime;
}

/*
 * Application key
 */
char *
application_key(app_t *app)
{
    char *app_key = NULL;
    tree_get_str(app->child.ptr[TREE_CONFIG], "app.key", &app_key);

    return app_key;
}

/*
 * Application domainname
 */
const char *
application_domainname(app_t *app)
{
    static char domainname[128];
    char *domain = NULL;

    tree_get_str(app->child.ptr[TREE_CONFIG], "app.domain", &domain);

    if (domain == NULL)
        return NULL;

    strcpy(domainname, application_instance());
    if (domain[0] != '.')
        strcat(domainname,  ".");
    strcat(domainname, domain);

    return strtolower(domainname);
}

/*
 * Release application
 */
void
application_release(app_t *app)
{
    if (app == NULL)
        return;

    /* Traverse tree and free pointers */
    tree_free(app);

    kore_free(app);
}
