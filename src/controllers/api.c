/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef __FreeBSD__
#define _POSIX_SOURCE //TODO: for the moment
#endif

#include <ayahesa.h>

#include <time.h>

/**
 * Accept array of strings and echo the contents.
 *
 * @param  array
 * @return array
 */
jrpc_method(echo)
{
    unsigned int i;

    if (!YAJL_IS_ARRAY(request->params)) {
        jsonrpc_log(request, LOG_ERR, "Echo only accepts positional params");
        return jsonrpc_error(request, JSONRPC_INVALID_PARAMS, NULL);
    }

    for (i = 0; i < request->params->u.array.len; i++) {
        yajl_val v = request->params->u.array.values[i];
        if (!YAJL_IS_STRING(v)) {
            jsonrpc_log(request, -3, "Echo only accepts strings");
            return jsonrpc_error(request, JSONRPC_INVALID_PARAMS, NULL);
        }
    }

    return jsonrpc_result(request, jrpc_write_string_array_params, NULL);
}

/**
 * Displays date and time according to parameters.
 *
 * @param  object|none
 * @return string
 */
jrpc_method(date)
{
    time_t		time_value;
    struct tm	time_info;
    char		timestamp[33];
    struct tm	*(*gettm)(const time_t *, struct tm *) = localtime_r;

    if (YAJL_IS_OBJECT(request->params)) {
        const char	*path[] = {"local", NULL};
        yajl_val	bf;

        bf = yajl_tree_get(request->params, path, yajl_t_false);
        if (bf != NULL)
            gettm = gmtime_r;
    } else if (request->params != NULL) {
        jsonrpc_log(request, LOG_ERR, "Date only accepts named params");
        return jsonrpc_error(request, JSONRPC_INVALID_PARAMS, NULL);
    }

    if ((time_value = time(NULL)) == -1)
        return jsonrpc_error(request, -2, "Failed to get date time");
    
    if (gettm(&time_value, &time_info) == NULL)
        return jsonrpc_error(request, -3, "Failed to get date time info");
    
    memset(timestamp, 0, sizeof(timestamp));
    if (strftime(timestamp, sizeof(timestamp) - 1, "%c", &time_info) == 0)
        return jsonrpc_error(request, -4, "Failed to get printable date time");

    return jsonrpc_result(request, jrpc_write_string, timestamp);
}

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
