/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include <assert.h>
#include <time.h>

#include "ayahesa.h"

#define TREE_CONFIG  0
#define TREE_CACHE  1

enum {
    ENOROOT = -1,
    EISFULL = -2,
};

static void
tree_new_root(struct app_tree *node)
{
    assert(node != NULL);

    node->child.alloc_cnt = 10;
    node->child.ptr = (struct app_tree **)calloc(node->child.alloc_cnt, sizeof(struct app_tree *));
}

static int
tree_get_new_index(struct app_tree *node)
{
    unsigned int i;

    assert(node != NULL);

    /* Return with error noroot */
    if (node->child.alloc_cnt == 0)
        return ENOROOT;

    for (i=0; i<node->child.alloc_cnt; ++i) {
        if (node->child.ptr[i] == NULL)
            return i;
    }

    /* Return with error isfull */
    return EISFULL;
}

static int
tree_expand(struct app_tree *node)
{
    unsigned int i, start_idx = node->child.alloc_cnt;

    assert(node != NULL);

    node->child.alloc_cnt += 10;
    node->child.ptr = (struct app_tree **)realloc(node->child.ptr, node->child.alloc_cnt * sizeof(struct app_tree *));
    
    /* Nullify new pointers */
    for (i=start_idx; i<node->child.alloc_cnt; ++i)
        node->child.ptr[i] = NULL;

    return start_idx;
}

static void
tree_free(struct app_tree *node)
{
    unsigned int i; 

    /* Skip empty nodes */
    if (node == NULL)
        return;

    if (node->key != NULL) {
        free((void *)node->key);
        node->key = NULL;
    }

    switch (node->type) {
        case T_STRING:
            free((void *)node->value.str);
            node->value.str = NULL;
            break;
        case T_POINTER:
            free(node->value.ptr);
            node->value.ptr = NULL;
            break;
        case T_INT:
        case T_NULL:
        case T_FLOAT:
        default:
            break;
    }

    /* Return when there is no subtree */
    if (node->child.ptr == NULL)
        return;

    /* Traverse down */
    for (i=0; i<node->child.alloc_cnt; ++i) {
        if (node->child.ptr[i] != NULL)
            tree_free(node->child.ptr[i]);
    }
}

/*
 * Bootstrap the application
 */
void
application_create(app_t **app)
{
    assert(*app == NULL);

    srand(time(NULL));

    /* Setup application root tree */
    app_t *root = (app_t *)calloc(1, sizeof(app_t));
    root->value.str =  generate_instance_id();
    tree_new_root(root);

    /* Config tree */
    root->child.ptr[TREE_CONFIG] = (app_t *)calloc(1, sizeof(app_t));
    tree_new_root(root->child.ptr[TREE_CONFIG]);

    /* Cache tree */
    root->child.ptr[TREE_CACHE] = (app_t *)calloc(1, sizeof(app_t));
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

    free((void *)app);
}

/*
 * Remove item from cache tree
 */
void
cache_remove(app_t *app, const char *key)
{
    unsigned int i;

    assert(app != NULL && app->child.ptr[TREE_CACHE]);

    struct app_tree *config_root = app->child.ptr[TREE_CACHE]; 

    for (i=0; i<config_root->child.alloc_cnt; ++i) {
        if (config_root->child.ptr[i] != NULL && !strcmp(config_root->child.ptr[i]->key, key)) {
            tree_free(config_root->child.ptr[i]);

            free((void *)config_root->child.ptr[i]);
            config_root->child.ptr[i] = NULL;
        }
    }
}

/*
 * Put string in cache tree
 */
void
cache_put_str(app_t *app, const char *key, const char *value)
{
    assert(app != NULL && app->child.ptr[TREE_CACHE]);

    struct app_tree *config_root = app->child.ptr[TREE_CACHE]; 

    /* Retrieve index */
    int idx = tree_get_new_index(config_root);
    if (idx == ENOROOT)
        abort();
    
    if (idx == EISFULL)
        idx = tree_expand(config_root);
printf("i:%d\n", idx);

    config_root->child.ptr[idx] = (app_t *)calloc(1, sizeof(app_t));
    config_root->child.ptr[idx]->key = strdup(key);
    config_root->child.ptr[idx]->value.str = strdup(value);
    config_root->child.ptr[idx]->type = T_STRING; 
}

/*
 * Put integer in cache tree
 */
void
cache_put_int(app_t *app, const char *key, int value)
{
    assert(app != NULL && app->child.ptr[TREE_CACHE]);

    struct app_tree *config_root = app->child.ptr[TREE_CACHE]; 

    /* Retrieve index */
    int idx = tree_get_new_index(config_root);
    if (idx == ENOROOT)
        abort();
    
    if (idx == EISFULL)
        idx = tree_expand(config_root);
printf("i:%d\n", idx);

    config_root->child.ptr[idx] = (app_t *)calloc(1, sizeof(app_t));
    config_root->child.ptr[idx]->key = strdup(key);
    config_root->child.ptr[idx]->value.i = value;
    config_root->child.ptr[idx]->type = T_INT; 
}
