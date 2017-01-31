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

#define TEST_AUD "C1M0L0P2"

/* Validate user claim, return object id on success */
static int
validate_claim(char *user, char *secret) {
    //TODO: replace stub with persistent storage validation
    if (!strcmp(user, "woei@quenza.net") && !strcmp(secret, "eZXn0haqu4sksxu2L9puUZNBCwqE"))
        return 110217;

    return 0;
}

static int
jrpc_write_jwt(struct jsonrpc_request *req, void *ctx)
{
	const unsigned char *str = (unsigned char *)ctx;

    yajl_gen_map_open(req->gen);
    yajl_gen_string(req->gen, (const unsigned char *)"token_type", 10);
    yajl_gen_string(req->gen, (const unsigned char *)"Bearer", 6);
    yajl_gen_string(req->gen, (const unsigned char *)"expires_in", 10);
    yajl_gen_integer(req->gen, app_session_lifetime());
    yajl_gen_string(req->gen, (const unsigned char *)"token", 5);
    yajl_gen_string(req->gen, str, strlen((const char *)str));

	return yajl_gen_map_close(req->gen);
}


/**
 * Authenticate principal and return JWT on
 * success.
 *
 * @param  object
 * @return string
 */
jrpc_method(jwt_authenticate)
{
    struct jwt jwt;
    yajl_val obj_user;
    yajl_val obj_secret;
    int object_id;

    if (YAJL_IS_OBJECT(request->params)) {
        const char	*path_user[] = {"user", NULL};
        const char	*path_secret[] = {"secret", NULL};

        obj_user = yajl_tree_get(request->params, path_user, yajl_t_string);
        obj_secret = yajl_tree_get(request->params, path_secret, yajl_t_string);
    } else {
        jsonrpc_log(request, LOG_ERR, "Authentication requires named parameters");
        return jsonrpc_error(request, JSONRPC_INVALID_PARAMS, NULL);
    }

    if (!YAJL_IS_STRING(obj_user)) 
        return jsonrpc_error(request, -2, "Parameter user requires string");

    if (!YAJL_IS_STRING(obj_secret)) 
        return jsonrpc_error(request, -3, "Parameter secret requires string");

    /* Generate new JWT token */
    if (!(object_id = validate_claim(YAJL_GET_STRING(obj_user), YAJL_GET_STRING(obj_secret)))) {

        /* Fire failed auth event */
        fire(EVENT_AUTH_FAILED, YAJL_GET_STRING(obj_user));

        return jsonrpc_error(request, -4, "Authentication failed");
    }

    /* Prepare token fields */
    jwt.sub = YAJL_GET_STRING(obj_user);
    jwt.aud = TEST_AUD;
    jwt.oid = object_id;

    /* Fire success auth event */
    fire(EVENT_AUTH_SUCCESS, jwt.sub);

    char *token = jwt_token_new(&jwt);
    int ret = jsonrpc_result(request, jrpc_write_jwt, token);
    kore_free(token);

    return ret;
}

/**
 * Request new JWT with refresh token
 * and old JWT.
 *
 * @param  object
 * @return string
 */
jrpc_method(jwt_refresh)
{
    return jsonrpc_error(request, -5, "Not Implemented");
}

/**
 * Revoke token and session.
 *
 * @param  object
 * @return string
 */
jrpc_method(jwt_revoke)
{
    return jsonrpc_error(request, -5, "Not Implemented");
}
