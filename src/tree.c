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
 * Put item in tree
 */
struct app_tree * 
tree_store(struct app_tree *tree)
{
    assert(tree != NULL);

    /* Retrieve index */
    int idx = tree_get_new_index(tree);
    if (idx == ENOROOT)
        abort();
    
    if (idx == EISFULL)
        idx = tree_expand(tree);
printf("i:%d\n", idx);//TODO: remove

    tree->child.ptr[idx] = (struct app_tree *)kore_malloc(sizeof(struct app_tree));
    tree->child.ptr[idx]->key = NULL;
    tree->child.ptr[idx]->type = T_NULL;
    return tree->child.ptr[idx];
}
