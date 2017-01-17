/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include <ayahesa.h>

#include "module.h"

#define TEST_AUD    MOD_MESSAGE MOD_PROJECTMGT MOD_OBJECTLIB

static int validate_claim(char *user, char *secret);

/**
 * Authenticate principal and return JWT on
 * success.
 *
 * @param  object|none
 * @return string
 */
jrpc_method(authenticate)
{
    yajl_val obj_user;
    yajl_val obj_secret;

    if (YAJL_IS_OBJECT(request->params)) {
        const char	*path_user[] = {"user", NULL};
        const char	*path_secret[] = {"secret", NULL};

        obj_user = yajl_tree_get(request->params, path_user, yajl_t_string);
        obj_secret = yajl_tree_get(request->params, path_secret, yajl_t_string);
        
        // printf("Principal: %s\n", YAJL_GET_STRING(obj_user));
        // printf("Secret: %s\n", YAJL_GET_STRING(obj_secret));
    } else {
        jsonrpc_log(request, LOG_ERR, "Authentication requires named parameters");
        return jsonrpc_error(request, JSONRPC_INVALID_PARAMS, NULL);
    }

    if (!YAJL_IS_STRING(obj_user)) 
        return jsonrpc_error(request, -2, "Parameter user requires string");

    if (!YAJL_IS_STRING(obj_secret)) 
        return jsonrpc_error(request, -3, "Parameter secret requires string");

    /* Generate new JWT token */
    if (!validate_claim(YAJL_GET_STRING(obj_user), YAJL_GET_STRING(obj_secret)))
        return jsonrpc_error(request, -4, "Authentication failed");

    char *token = jwt_token_new(YAJL_GET_STRING(obj_user), TEST_AUD);
    int ret = jsonrpc_result(request, jrpc_write_string, token);
    kore_free(token);

    return ret;
}

/* Validate user claim, return 0 on success */
static int
validate_claim(char *user, char *secret) {
    if (!strcmp(user, "woei") && !strcmp(secret, "eZXn0haqu4sksxu2L9puUZNBCwqE"))
        return 1;

    return 0;
}
