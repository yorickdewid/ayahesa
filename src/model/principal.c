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

//TODO: change
#include "../include/ayahesa.h"
#include "../include/argon2.h"
#include "../core/util.h"

#define T_COST  2
#define M_COST  2^10
#define P_COST  1

int			model_principal_auth(const char *, char *);

static char *
password_hash(const char *secret)
{
    unsigned char salt[16];
    char *encoded;
    size_t encodedlen;

    /* Generate salt */
    random_string(salt, 16, 1);

    /* Calculate length */
    encodedlen = argon2_encodedlen(T_COST, M_COST, P_COST, 16, 32, Argon2_i);
    encoded = (char *)kore_malloc(encodedlen);

    /* Calculate hash */
    if (argon2i_hash_encoded(T_COST, M_COST, P_COST, secret, strlen(secret),
                            salt, 16,
                            32, encoded, encodedlen) != ARGON2_OK) {
        return NULL;
    }

    return encoded;
}

int
model_principal_auth(const char *user, char *secret)
{
	struct kore_pgsql	pgsql;
	char		    	*id, *password;
    int                 object_id = 0;
    char                sqlbuffer[256];

    //TODO: prepared statement
    const char selectsql[] =
        "SELECT id,password "
        "FROM users "
        "WHERE email='%s' "
        "LIMIT 1";

    const char updatesql[] =
        "UPDATE users "
        "SET password='%s' "
        "WHERE id='%d'";

    /* Initialize connection */
	if (!kore_pgsql_query_init(&pgsql, NULL, "dbrw", KORE_PGSQL_SYNC)) {
		kore_pgsql_logerror(&pgsql);
        goto done;
	}

    /* Execute query */
    snprintf(sqlbuffer, 256, selectsql, user);
	if (!kore_pgsql_query(&pgsql, sqlbuffer)) {
		kore_pgsql_logerror(&pgsql);
        goto done;
	}

    /* No match */
    if (!kore_pgsql_ntuples(&pgsql))
        goto done;
    
    /* Fetch password and validate */
	password = kore_pgsql_getvalue(&pgsql, 0, 1);
    if (argon2i_verify(password, secret, strlen(secret)) != ARGON2_OK)
        goto done;

    /* Fetch object id */
	id = kore_pgsql_getvalue(&pgsql, 0, 0);
    object_id = atoi(id);

    /* Generate new password hash */
    char *password_hash_new = password_hash(secret);

    /* Update password hash */
    snprintf(sqlbuffer, 256, updatesql, password_hash_new, object_id);
    kore_free(password_hash_new);
	if (!kore_pgsql_query(&pgsql, sqlbuffer)) {
		kore_pgsql_logerror(&pgsql);
        goto done;
	}

done:
	/* All good */
	kore_pgsql_cleanup(&pgsql);

	return object_id;
}
