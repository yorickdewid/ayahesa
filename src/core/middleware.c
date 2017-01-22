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

int middleware_session(struct http_request *req, char *data);

int
middleware_session(struct http_request *req, char *data)
{
    char *method = strtok(data, " ");
    if (method == NULL)
        return KORE_RESULT_ERROR;

	/* Require bearer authentication */
    if (!strcmp(strtolower(method), "bearer")) {
        char *token = strtok(NULL, " ");

		/* Verify token */
		if (jwt_verify(token))
			return KORE_RESULT_OK;
    }

	return KORE_RESULT_ERROR;
}
