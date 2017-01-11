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

void
tree_new_root(struct app_tree *node)
{
    unsigned int i;

    assert(node != NULL);

    node->child.alloc_cnt = 10;
    node->child.ptr = (struct app_tree **)kore_malloc(node->child.alloc_cnt * sizeof(struct app_tree *));

    /* Nullify new pointers */
    for (i=0; i<node->child.alloc_cnt; ++i)
        node->child.ptr[i] = NULL;
}

int
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

int
tree_expand(struct app_tree *node)
{
    unsigned int i, start_idx = node->child.alloc_cnt;

    assert(node != NULL);

    node->child.alloc_cnt += 10;
    node->child.ptr = (struct app_tree **)kore_realloc(node->child.ptr, node->child.alloc_cnt * sizeof(struct app_tree *));
    
    /* Nullify new pointers */
    for (i=start_idx; i<node->child.alloc_cnt; ++i)
        node->child.ptr[i] = NULL;

    return start_idx;
}

void
tree_free(struct app_tree *node)
{
    unsigned int i; 

    /* Skip empty nodes */
    if (node == NULL)
        return;

    if (node->key != NULL) {
        kore_free((void *)node->key);
        node->key = NULL;
    }

    switch (node->type) {
        case T_STRING:
            kore_free((void *)node->value.str);
            node->value.str = NULL;
            break;
        case T_POINTER: /* Assume the pointer is freed */
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
 * Remove item from tree
 */
void
tree_remove(struct app_tree *tree, const char *key)
{
    unsigned int i;

    assert(tree != NULL);

    if (tree->flags & T_FLAG_READONLY) {
        return;
    }
 
    for (i=0; i<tree->child.alloc_cnt; ++i) {
        if (tree->child.ptr[i] != NULL && !strcmp(tree->child.ptr[i]->key, key)) {
            tree_free(tree->child.ptr[i]);

            kore_free((void *)tree->child.ptr[i]);
            tree->child.ptr[i] = NULL;
        }
    }
}

/*
 * Allocate new item in tree
 */
static struct app_tree *
tree_new_item(struct app_tree *tree)
{
    assert(tree != NULL);

    /* Retrieve index */
    int idx = tree_get_new_index(tree);
    if (idx == ENOROOT)
        abort();
    
    if (idx == EISFULL)
        idx = tree_expand(tree);

    tree->child.ptr[idx] = (struct app_tree *)kore_malloc(sizeof(struct app_tree));
    tree->child.ptr[idx]->key = NULL;
    tree->child.ptr[idx]->type = T_NULL;
    return tree->child.ptr[idx];
}

/*
 * Put integer in cache tree
 */
void
tree_put_int(struct app_tree *tree, const char *key, int value)
{
    assert(tree != NULL);

    struct app_tree *item = tree_new_item(tree);

    item->key = kore_strdup(key);
    item->value.i = value;
    item->type = T_INT;
}

/*
 * Put float in cache tree
 */
void
tree_put_float(struct app_tree *tree, const char *key, float value)
{
    assert(tree != NULL);

    struct app_tree *item = tree_new_item(tree);

    item->key = kore_strdup(key);
    item->value.f = value;
    item->type = T_FLOAT;
}

/*
 * Put string in cache tree
 */
void
tree_put_str(struct app_tree *tree, const char *key, const char *value)
{
    assert(tree != NULL);

    struct app_tree *item = tree_new_item(tree);

    item->key = kore_strdup(key);
    item->value.str = kore_strdup(value);
    item->type = T_STRING; 
}

/*
 * Put user pointer in cache tree
 */
void
tree_put_ptr(struct app_tree *tree, const char *key, void *value)
{
    assert(tree != NULL);

    struct app_tree *item = tree_new_item(tree);

    item->key = kore_strdup(key);
    item->value.str = value;
    item->type = T_POINTER; 
}
