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

enum {
    ENOROOT = -1,
    EISFULL = -2,
};

void tree_new_root(struct app_tree *node);
int tree_get_new_index(struct app_tree *node);
int tree_expand(struct app_tree *node);
void tree_free(struct app_tree *node);

struct app_tree *tree_store(struct app_tree *tree);

#endif // _AYAHESA_TREE_H_
