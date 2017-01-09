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

    char *cookie = NULL;
    http_request_header(request, "cookie", &cookie);

    if (cookie)
        puts(cookie);

    puts(app_instance_id());

	http_response_header(request, "content-type", "text/html");
	//http_response_header(request, "set-cookie", "_umaysess=3745693");
	http_response(request, 200, "x", 1);
    return_ok();
}
