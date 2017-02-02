/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

/*
 * This example demonstrates how to use synchronous PGSQL queries
 * with Kore. For an asynchronous example see pgsql/ under examples/.
 *
 * This example does the same as the asynchronous one, select all entries
 * from a table called "coders".
 */

#include "../include/ayahesa.h"

int			model_principal_auth(const char *, const char *);

int
model_principal_auth(const char *user, const char *secret)
{
	struct kore_pgsql	pgsql;
	char		    	*id;
    int                 object_id = 0;
    char                sqlbuffer[256];

    //TODO: prepared statement
    const char sql[] =
        "SELECT id "
        "FROM users "
        "WHERE email='%s' "
        "AND "
        "password='%s' "
        "LIMIT 1";

    /* Initialize connection */
	if (!kore_pgsql_query_init(&pgsql, NULL, "dbr", KORE_PGSQL_SYNC)) {
		kore_pgsql_logerror(&pgsql);
        goto done;
	}

    /* Execute query */
    snprintf(sqlbuffer, 256, sql, user, secret);
	if (!kore_pgsql_query(&pgsql, sqlbuffer)) {
		kore_pgsql_logerror(&pgsql);
        goto done;
	}

    /* No match */
    if (!kore_pgsql_ntuples(&pgsql))
        goto done;

	id = kore_pgsql_getvalue(&pgsql, 0, 0);
    object_id = atoi(id);

done:
	/* All good */
	kore_pgsql_cleanup(&pgsql);

	return object_id;
}
