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

#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>

void        db_init(void);
int			db_user(void);

/* Called when our module is loaded (see config) */
void
db_init(void)
{
	/* Register our database. */
	kore_pgsql_register("db", "host=localhost dbname=ayahesa user=postgres password=q");
}

/* Page handler entry point (see config) */
int
db_user(void)
{
	struct kore_pgsql	sql;
	char			*name;
	int			rows, i;

	// req->status = HTTP_STATUS_INTERNAL_ERROR;

	/*
	 * Initialise our kore_pgsql data structure with the database name
	 * we want to connect to (note that we registered this earlier with
	 * kore_pgsql_register()). We also say we will perform a synchronous
	 * query (KORE_PGSQL_SYNC) and we do not need to pass our http_request
	 * so we pass NULL instead.
	 */
	if (!kore_pgsql_query_init(&sql, NULL, "db", KORE_PGSQL_SYNC)) {
		kore_pgsql_logerror(&sql);
        return 0;
		//goto out;
	}

	/*
	 * Now we can fire off the query, once it returns we either have
	 * a result on which we can operate or an error occured.
	 */
	if (!kore_pgsql_query(&sql, "SELECT * FROM users")) {
		kore_pgsql_logerror(&sql);
        return 0;
		//goto out;
	}

	/*
	 * Iterate over the result and dump it to somewhere.
	 */
	rows = kore_pgsql_ntuples(&sql);
	for (i = 0; i < rows; i++) {
		name = kore_pgsql_getvalue(&sql, i, 1);
		kore_log(LOG_NOTICE, "name: '%s'", name);
	}

	/* All good. */
	// req->status = HTTP_STATUS_OK;

//out:
//	http_response(req, req->status, NULL, 0);

	/* Don't forget to cleanup the kore_pgsql data structure. */
	kore_pgsql_cleanup(&sql);

	return 1;
}
