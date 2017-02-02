/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef _AYAHESA_TREE_H_
#define _AYAHESA_TREE_H_

#define TREE_CONFIG  0
#define TREE_CACHE  1

/*
 * Config operations
 */

#define config_put_int(a,k,v) \
    tree_put_int(a->child.ptr[TREE_CONFIG],k,v)

#define config_put_float(a,k,v) \
    tree_put_float(a->child.ptr[TREE_CONFIG],k,v)

#define config_put_str(a,k,v) \
    tree_put_str(a->child.ptr[TREE_CONFIG],k,v)

#define config_put_ptr(a,k,v) \
    tree_put_ptr(a->child.ptr[TREE_CONFIG],k,v)


#define config_get_int(a,k,v) \
    tree_get_int(a->child.ptr[TREE_CONFIG],k,v)

#define config_get_float(a,k,v) \
    tree_get_float(a->child.ptr[TREE_CONFIG],k,v)

#define config_get_str(a,k,v) \
    tree_get_str(a->child.ptr[TREE_CONFIG],k,v)

#define config_get_ptr(a,k,v) \
    tree_get_ptr(a->child.ptr[TREE_CONFIG],k,v)


#define config_remove(a,k) \
    tree_remove(a->child.ptr[TREE_CONFIG], k);

/*
 * Cache operations
 */

#define cache_put_int(a,k,v) \
    tree_put_int(a->child.ptr[TREE_CACHE],k,v)

#define cache_put_float(a,k,v) \
    tree_put_float(a->child.ptr[TREE_CACHE],k,v)

#define cache_put_str(a,k,v) \
    tree_put_str(a->child.ptr[TREE_CACHE],k,v)

#define cache_put_ptr(a,k,v) \
    tree_put_ptr(a->child.ptr[TREE_CACHE],k,v)


#define cache_get_int(a,k,v) \
    tree_get_int(a->child.ptr[TREE_CACHE],k,v)

#define cache_get_float(a,k,v) \
    tree_get_float(a->child.ptr[TREE_CACHE],k,v)

#define cache_get_str(a,k,v) \
    tree_get_str(a->child.ptr[TREE_CACHE],k,v)

#define cache_get_ptr(a,k,v) \
    tree_get_ptr(a->child.ptr[TREE_CACHE],k,v)


#define cache_remove(a,k) \
    tree_remove(a->child.ptr[TREE_CACHE], k);

enum {
    ENOROOT = -1,
    EISFULL = -2,
};

void tree_new_root(struct app_tree *node);
int tree_get_new_index(struct app_tree *node);
int tree_expand(struct app_tree *node);
void tree_free(struct app_tree *node);

int tree_contains(struct app_tree *tree, const char *key);

void tree_remove(struct app_tree *tree, const char *key);

void tree_put_int(struct app_tree *tree, const char *key, int value);
void tree_put_float(struct app_tree *tree, const char *key, float value);
void tree_put_str(struct app_tree *tree, const char *key, const char *value);
void tree_put_ptr(struct app_tree *tree, const char *key, void *value);

void tree_get_int(struct app_tree *tree, const char *key, int *value);
void tree_get_float(struct app_tree *tree, const char *key, float *value);
void tree_get_str(struct app_tree *tree, const char *key, char **value);
void tree_get_ptr(struct app_tree *tree, const char *key, void **value);

// #ifdef DEBUG
void tree_dump(struct app_tree *node);
// #endif // DEBUG

#endif // _AYAHESA_TREE_H_
