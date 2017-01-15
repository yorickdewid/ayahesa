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
route(root) { redirect("/app/main"); }

/*
 * Welcome page, show some framework love
 */
route(main)
{
	const char *default_page = 
		"<html>"
		"<head><title>Welcome</title></head>"
		"<body>"
		"<h1>Welcome to Ayahesa</h1>"
		"</body>"
		"</html>";

    /* Only accept GET requests */
	http_get();

	http_response_header(request, "content-type", "text/html");
	http_response_header(request, "set-cookie", "_umaysess=3745693");
	http_response(request, 200, default_page, strlen(default_page));

	/* We're good */
	return_ok();
}

/*
 * Handle request in controller
 *
 * Controller: foo
 */
route(foo)
{
    invoke(foo);

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

#if 0
/*
 * Welcome page, show some framework love
 */
route(chat)
{
    /* Only accept GET requests */
	http_get();

	http_response_header(request, "content-type", "text/html");
	http_response(request, 200, asset_chat_html, asset_len_chat_html);

	/* We're good */
	return_ok();
}

/*
 * Welcome page, show some framework love
 */
route(chat_js)
{
    /* Only accept GET requests */
	http_get();

	http_response_header(request, "content-type", "text/javascript");
	http_response(request, 200, asset_app_js, asset_len_app_js);

	/* We're good */
	return_ok();
}

/*
 * Welcome page, show some framework love
 */
route(chat_css)
{
    /* Only accept GET requests */
	http_get();

	http_response_header(request, "content-type", "text/css");
	http_response(request, 200, asset_style_css, asset_len_style_css);

	/* We're good */
	return_ok();
}
#endif

void		websocket_connect(struct connection *);
void		websocket_disconnect(struct connection *);
void		websocket_message(struct connection *, u_int8_t, void *, size_t);

/* Websocket callbacks. */
struct kore_wscbs wscbs = {
	websocket_connect,
	websocket_message,
	websocket_disconnect
};

/* Called whenever we get a new websocket connection. */
void
websocket_connect(struct connection *c)
{
	// kore_log(LOG_NOTICE, "%p: connected", c);
	kore_websocket_broadcast(c, WEBSOCKET_OP_TEXT, "New user joins chat", 19, WEBSOCKET_BROADCAST_LOCAL);
}

void
websocket_message(struct connection *c, u_int8_t op, void *data, size_t len)
{
	kore_websocket_broadcast(c, op, data, len, WEBSOCKET_BROADCAST_LOCAL);
	// kore_websocket_send(c, WEBSOCKET_OP_TEXT, data, len);
}

void
websocket_disconnect(struct connection *c)
{
	// kore_log(LOG_NOTICE, "%p: disconnecting", c);
	kore_websocket_broadcast(c, WEBSOCKET_OP_TEXT, "User left chat", 14, WEBSOCKET_BROADCAST_LOCAL);
}

/*
 * Message socket
 */
endpoint(msg)
{
	/* Perform the websocket handshake, passing our callbacks. */
	kore_websocket_handshake(_request, &wscbs);

	/* We're good */
	return_ok();
}
