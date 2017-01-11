/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include "ayahesa.h"
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

    /*config_put_str(app, "dbname", "ayahesa");
    config_put_str(app, "dbuser", "user");
    config_put_str(app, "dbpasswd", "ABC@123");
    config_put_str(app, "dbhost", "localhost");*/

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

/*
 * Remove item from cache tree
 */
void
cache_remove(app_t *app, const char *key)
{
    unsigned int i;

    assert(app != NULL && app->child.ptr[TREE_CACHE]);

    struct app_tree *cache_root = app->child.ptr[TREE_CACHE]; 

    for (i=0; i<cache_root->child.alloc_cnt; ++i) {
        if (cache_root->child.ptr[i] != NULL && !strcmp(cache_root->child.ptr[i]->key, key)) {
            tree_free(cache_root->child.ptr[i]);

            kore_free((void *)cache_root->child.ptr[i]);
            cache_root->child.ptr[i] = NULL;
        }
    }
}

#if 0
/*
 * Put string in config tree
 */
void
config_put_str(app_t *app, const char *key, const char *value)
{
    assert(app != NULL && app->child.ptr[TREE_CONFIG]);

    struct app_tree *config_root = app->child.ptr[TREE_CONFIG]; 

    /* Retrieve index */
    int idx = tree_get_new_index(config_root);
    if (idx == ENOROOT)
        abort();
    
    if (idx == EISFULL)
        idx = tree_expand(config_root);
printf("i:%d\n", idx);

    config_root->child.ptr[idx] = (app_t *)kore_malloc(sizeof(app_t));
    config_root->child.ptr[idx]->key = kore_strdup(key);
    config_root->child.ptr[idx]->value.str = kore_strdup(value);
    config_root->child.ptr[idx]->type = T_STRING; 
}
#endif

/*
 * Put integer in cache tree
 */
void
cache_put_int(app_t *app, const char *key, int value)
{
    assert(app->child.ptr[TREE_CACHE]);

    struct app_tree *cache_item = tree_store(app->child.ptr[TREE_CACHE]);

    cache_item->key = kore_strdup(key);
    cache_item->value.i = value;
    cache_item->type = T_INT;
}

/*
 * Put float in cache tree
 */
void
cache_put_float(app_t *app, const char *key, float value)
{
    assert(app->child.ptr[TREE_CACHE]);

    struct app_tree *cache_item = tree_store(app->child.ptr[TREE_CACHE]);

    cache_item->key = kore_strdup(key);
    cache_item->value.f = value;
    cache_item->type = T_FLOAT;
}

/*
 * Put string in cache tree
 */
void
cache_put_str(app_t *app, const char *key, const char *value)
{
    assert(app->child.ptr[TREE_CACHE]);

    struct app_tree *cache_item = tree_store(app->child.ptr[TREE_CACHE]);

    cache_item->key = kore_strdup(key);
    cache_item->value.str = kore_strdup(value);
    cache_item->type = T_STRING; 
}

/*
 * Put user pointer in cache tree
 */
void
cache_put_ptr(app_t *app, const char *key, void *value)
{
    assert(app->child.ptr[TREE_CACHE]);

    struct app_tree *cache_item = tree_store(app->child.ptr[TREE_CACHE]);

    cache_item->key = kore_strdup(key);
    cache_item->value.str = value;
    cache_item->type = T_POINTER; 
}

