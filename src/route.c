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
|--------------------------------------------------------------------------
| Web Routes & Endpoint Routes
|--------------------------------------------------------------------------
|
| Here is where you can register web routes for your application. These
| routes are called when a configuration URI is hit. Keep the routes
| simple and move any operations or actions into a controller or unit 
| method. Now create something great!
|
*/

#include <ayahesa.h>

#include "assets.h"

/*
 * Default routes with quick return
 */
route(report) { write_plain("Reporting back"); }
route(html) { write_html("<b>Important!</b>"); }
route(root) { redirect("/app/welcome"); }

/*
 * Welcome page, show some framework love
 */
route(welcome)
{
	const char *default_page = 
		"<html>"
		"<head><title>Welcome to Ayahesa</title></head>"
		"<body style=\"background-color:#333;color:#fff;\">"
		"<div style=\"font-family:sans-serif;text-align:center;margin:260px;\">"
		"<span style=\"font-size:50px;\">Welcome to Ayahesa</span>"
		"</div>"
		"</body>"
		"</html>";

    /* Only accept GET requests */
	http_get();

	http_response_header(request, "content-type", "text/html");
	http_response(request, 200, default_page, strlen(default_page));

	/* We're good */
	return_ok();
}

/*
 * Handle request in controller
 *
 * Controller: foo
 */
route(cookie)
{
    /* Only accept GET requests */
	http_get();

    invoke(foo);

	/* We're good */
	return_ok();
}

/*
 * Handle request in controller
 *
 * Controller: resource
 */
route(resource)
{
    invoke(resource);

	/* We're good */
	return_ok();
}

/*
 * JSON-RPC Application core endpoint
 *
 * Controller: endpoint_core
 */
endpoint(core)
{
	jrpc_parse();

	/* Endpoint info */
	jrpc_info("Application core functions");

	/* Call controller */
	jrpc_invoke("echo", echo);
	jrpc_invoke("date", date);

	/* No method found */
	jrpc_return_error();
}

/*
 * JSON-RPC Auth endpoint
 *
 * Controller: endpoint_auth
 */
endpoint(auth)
{
	jrpc_parse();

	/* Endpoint info */
	jrpc_info("Authentication and authorization functions");

	/* Call controller */
	jrpc_invoke("authenticate", authenticate);

	/* No method found */
	jrpc_return_error();
}

/*
 * Message socket
 *
 * Controller: endpoint_msg
 */
endpoint(msg)
{
	invoke(msg);

	/* We're good */
	return_ok();
}
