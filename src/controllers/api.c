/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#define _POSIX_SOURCE //TODO: for the moment
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
