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

#include <assert.h>
#include <time.h>

/*
 * Bootstrap the application
 */
void
application_create(app_t **app)
{
    assert(*app == NULL);

    /* Seed random pool */
    srand(time(NULL));

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
 * Parse application configuration
 */
void
application_config(app_t *app, const char *configfile)
{
    assert(app != NULL);

    /* Default config */
    config_put_int(app, "debug", 0);
    config_put_str(app, "env", "prod");

    cache_put_str(app, "appname", "Ayahesa");//0
    cache_put_str(app, "appkey1", "ABC@123");
    cache_put_str(app, "appkey2", "ABC@123");
    cache_put_str(app, "appkey3", "ABC@123");
    cache_put_str(app, "appkey4", "ABC@123");
    cache_put_str(app, "appkey5", "ABC@123");
    cache_put_str(app, "appkey6", "ABC@123");
    cache_put_str(app, "appkey7", "ABC@123");
    cache_put_str(app, "appkey8", "ABC@123");//8
    cache_put_str(app, "appkey9", "ABC@123");//9
    cache_put_str(app, "appkey10", "q");
    cache_put_str(app, "appkey11", "q");
    cache_remove(app, "appkey3");
    cache_put_str(app, "appkey12", "q");
    cache_put_int(app, "appkey16", 12);
}

/*
 * Release application
 */
void
application_release(app_t *app)
{
    if (app == NULL)
        return;

    tree_free(app);

    kore_free((void *)app);
}
