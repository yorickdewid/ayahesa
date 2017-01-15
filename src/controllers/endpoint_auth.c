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
    yajl_val obj_algorithm;

    if (YAJL_IS_OBJECT(request->params)) {
        const char	*path_user[] = {"user", NULL};
        const char	*path_secret[] = {"secret", NULL};
        const char	*path_algo[] = {"algorithm", NULL};

        obj_user = yajl_tree_get(request->params, path_user, yajl_t_string);
        obj_secret = yajl_tree_get(request->params, path_secret, yajl_t_string);
        obj_algorithm = yajl_tree_get(request->params, path_algo, yajl_t_string);
        
        printf("Principal: %s\n", YAJL_GET_STRING(obj_user));
        printf("Secret: %s\n", YAJL_GET_STRING(obj_secret));
        printf("Algorithm: %s\n", YAJL_GET_STRING(obj_algorithm));
    } else {
        jsonrpc_log(request, LOG_ERR, "Authentication requires named parameters");
        return jsonrpc_error(request, JSONRPC_INVALID_PARAMS, NULL);
    }

    if (!YAJL_IS_STRING(obj_user)) 
        return jsonrpc_error(request, -2, "Parameter user requires string");

    if (!YAJL_IS_STRING(obj_secret)) 
        return jsonrpc_error(request, -2, "Parameter secret requires string");

    if (!YAJL_IS_STRING(obj_algorithm)) 
        return jsonrpc_error(request, -2, "Parameter algorithm requires string");

    // Handle incomming info

    char *jwt = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"
        ".eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9"
        ".TJVA95OrM7E2cBab30RMHrHDcEfxjoYZgeFONFh7HgQ";

    return jsonrpc_result(request, jrpc_write_string, (void *)jwt);
}
