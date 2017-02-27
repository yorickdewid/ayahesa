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

#define T_COST  2
#define M_COST  2^10
#define P_COST  1

int	model_principal_auth(const char *, char *);//TODO

int
model_principal_auth(const char *user, char *secret)
{
    struct kore_pgsql   pgsql;
    char                *id, *password_hash;
    char                *password_hash_new;
    int                 object_id = 0;
    char                sqlbuffer[256];

    //TODO: prepared statement
    const char selectsql[] =
        "SELECT id,password "
        "FROM users "
        "WHERE email='%s' "
        "LIMIT 1";

    //TODO: prepared statement
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
    password_hash = kore_pgsql_getvalue(&pgsql, 0, 1);
    if (!crypt_password_verify(password_hash, secret))
        goto done;

    /* Fetch object id */
    id = kore_pgsql_getvalue(&pgsql, 0, 0);
    object_id = atoi(id);

    /* Generate new password hash */
    password_hash_new = crypt_password_hash(secret);

    /* Update password hash */
    snprintf(sqlbuffer, 256, updatesql, password_hash_new, object_id);
    aya_free(password_hash_new);
    if (!kore_pgsql_query(&pgsql, sqlbuffer)) {
        kore_pgsql_logerror(&pgsql);
        goto done;
    }

done:
    /* All good */
    kore_pgsql_cleanup(&pgsql);

    return object_id;
}
