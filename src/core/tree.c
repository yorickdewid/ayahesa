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
    assert(node != NULL);

    node->child.alloc_cnt = 10;
    node->child.ptr = (struct app_tree **)aya_calloc(node->child.alloc_cnt, sizeof(struct app_tree *));
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
        aya_free(node->key);
    }

    switch (node->type) {
        case T_STRING:
            aya_free(node->value.str);
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
        if (node->child.ptr[i] != NULL) {
            tree_free(node->child.ptr[i]);
            aya_free(node->child.ptr[i]);
        }
    }

    node->child.ptr = NULL;
}

/*
 * Check if tree contains key
 */
int
tree_contains(struct app_tree *tree, const char *key)
{
    unsigned int i;

    assert(tree != NULL);

    for (i=0; i<tree->child.alloc_cnt; ++i) {
        if (tree->child.ptr[i] != NULL && !strcmp(tree->child.ptr[i]->key, key)) {
            return 1;
        }
    }

    return 0;
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

            aya_free(tree->child.ptr[i]);
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

    tree->child.ptr[idx] = (struct app_tree *)aya_calloc(1, sizeof(struct app_tree));
    tree->child.ptr[idx]->key = NULL;
    tree->child.ptr[idx]->type = T_NULL;
    return tree->child.ptr[idx];
}

/*
 * Put integer in tree
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
 * Put float in tree
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
 * Put string in tree
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
 * Put pointer in tree
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

/*
 * Get integer in tree
 */
void
tree_get_int(struct app_tree *tree, const char *key, int *value)
{
    unsigned int i;

    assert(tree != NULL);

    for (i=0; i<tree->child.alloc_cnt; ++i) {
        if (tree->child.ptr[i] != NULL && !strcmp(tree->child.ptr[i]->key, key)) {
            if (tree->child.ptr[i]->type != T_INT)
                return;

            *value = tree->child.ptr[i]->value.i;
        }
    }
}

/*
 * Get float in tree
 */
void
tree_get_float(struct app_tree *tree, const char *key, float *value)
{
    unsigned int i;

    assert(tree != NULL);

    for (i=0; i<tree->child.alloc_cnt; ++i) {
        if (tree->child.ptr[i] != NULL && !strcmp(tree->child.ptr[i]->key, key)) {
            if (tree->child.ptr[i]->type != T_FLOAT)
                return;

            *value = tree->child.ptr[i]->value.f;
        }
    }
}

/*
 * Get string in tree
 */
void
tree_get_str(struct app_tree *tree, const char *key, char **value)
{
    unsigned int i;

    assert(tree != NULL);

    for (i=0; i<tree->child.alloc_cnt; ++i) {
        if (tree->child.ptr[i] != NULL && !strcmp(tree->child.ptr[i]->key, key)) {
            if (tree->child.ptr[i]->type != T_STRING)
                return;

            *value = tree->child.ptr[i]->value.str;
        }
    }
}

/*
 * Get pointer in tree
 */
void
tree_get_ptr(struct app_tree *tree, const char *key, void **value)
{
    unsigned int i;

    assert(tree != NULL);

    for (i=0; i<tree->child.alloc_cnt; ++i) {
        if (tree->child.ptr[i] != NULL && !strcmp(tree->child.ptr[i]->key, key)) {
            if (tree->child.ptr[i]->type != T_POINTER)
                return;

            *value = tree->child.ptr[i]->value.ptr;
        }
    }
}


// #ifdef DEBUG

void
tree_dump(struct app_tree *node)
{
    unsigned int i;

    /* Skip empty nodes */
    if (node == NULL)
        return;

    if (node->key != NULL)
        printf(">> %s -> ", node->key);

    switch (node->type) {
        case T_STRING:
            printf("%s(str)\n", node->value.str);
            break;
        case T_POINTER:
            printf("(ptr)\n");
            break;
        case T_INT:
            printf("%d(int)\n", node->value.i);
            break;
        case T_NULL:
            printf("(null)\n");
            break;
        case T_FLOAT:
            printf("%f(float)\n", node->value.f);
            break;
        default:
            printf("(?)\n");
            break;
    }

    /* Return when there is no subtree */
    if (node->child.ptr == NULL)
        return;

    printf("> %d children\n", node->child.alloc_cnt);

    /* Traverse down */
    for (i=0; i<node->child.alloc_cnt; ++i) {
        if (node->child.ptr[i] != NULL)
            tree_dump(node->child.ptr[i]);
    }
}

// #endif
