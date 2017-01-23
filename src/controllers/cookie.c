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

/**
 * Cookie controller
 *
 * Test the request information and set a cookie
 * when not done before. Report example text back.
 */
controller(cookie)
{
    const char *cookie = http_get_cookie(request, "track");
    if (cookie == NULL)
        http_response_header(request, "set-cookie", "track=1");

    http_response_header(request, "content-type", "text/html");
	http_response(request, 200, NULL, 0);

    return_ok();
}
