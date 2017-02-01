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

void database_init(void);

void
database_init(void)
{
	/* Register our database. */
	kore_pgsql_register("db", "host=localhost dbname=ayahesa user=postgres password=q");
}
