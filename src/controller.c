/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include "ayahesa.h"

controller(foo)
{
    if (request->host)
        puts(request->host);
    if (request->path)
        puts(request->path);
    if (request->agent)
        puts(request->agent);
    if (request->query_string)
        puts(request->query_string);

    const char *cookie = get_cookie(request, "authsess");

	http_response_header(request, "content-type", "text/html");
    if (cookie == NULL)
	    http_response_header(request, "set-cookie", "authsess=dyfgyasdfwegfgwegrfw4i");

	http_response(request, 200, "Blub!", 1);
    return_ok();
}
