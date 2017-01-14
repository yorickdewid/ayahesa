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
application_prelude(void)
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
    if (!strcmp(name, "app_name") || !strcmp(name, "app_env") || !strcmp(name, "app_key")) {
        if (!strlen(value)) {
            printf("Config item starting with 'app_' cannot be empty (line:%d)\n", lineno);
            abort();
        }
    }

    /* Redefine config */
    if (!strcmp(name, "debug")) {
        if (!strcmp(value, "true") || !strcmp(value, "1")) {
            config_put_int(app, "debug", 1);
         } else {
            config_put_int(app, "debug", 0);
         }
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
    int has_name = tree_contains(app->child.ptr[TREE_CONFIG], "app_name");
    int has_env = tree_contains(app->child.ptr[TREE_CONFIG], "app_env");
    int has_key = tree_contains(app->child.ptr[TREE_CONFIG], "app_key");
    int has_debug = tree_contains(app->child.ptr[TREE_CONFIG], "debug");

    /* No app_name */
    if (!has_name) {
        puts("'app_name' cannot be empty");
        abort();
    }

    /* No app_env */
    if (!has_env) {
        puts("'app_env' cannot be empty");
        abort();
    }

    /* No app_key */
    if (!has_key) {
        puts("'app_key' cannot be empty");
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
        printf("Framework configuration error: %s\n", configfile);
        abort();
    }

    /* Check and default config */
    validate_config(app);

    tree_dump(app->child.ptr[TREE_CONFIG]);
    tree_dump(app->child.ptr[TREE_CACHE]);
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
    tree_get_str(app->child.ptr[TREE_CONFIG], "app_name", &name);

    return name;
}

/*
 * Application environment
 */
char *
application_environment(app_t *app)
{
    char *name = NULL;
    tree_get_str(app->child.ptr[TREE_CONFIG], "app_env", &name);

    return name;
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

    kore_free((void *)app);
}
