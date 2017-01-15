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

static void websocket_connect(struct connection *);
static void websocket_disconnect(struct connection *);
static void websocket_message(struct connection *, u_int8_t, void *, size_t);

/* Websocket callbacks. */
struct kore_wscbs wscbs = {
	websocket_connect,
	websocket_message,
	websocket_disconnect
};

/**
 * Message controller
 *
 * Setup websocket server and register
 * callbacks.
 */
controller(msg)
{
    /* Perform the websocket handshake, passing our callbacks. */
	kore_websocket_handshake(request, &wscbs);
    return_ok();
}

/* Called whenever we get a new websocket connection. */
static void
websocket_connect(struct connection *c)
{
	// kore_log(LOG_NOTICE, "%p: connected", c);
	kore_websocket_broadcast(c, WEBSOCKET_OP_TEXT, "New user joins chat", 19, WEBSOCKET_BROADCAST_LOCAL);
}

static void
websocket_message(struct connection *c, u_int8_t op, void *data, size_t len)
{
	kore_websocket_broadcast(c, op, data, len, WEBSOCKET_BROADCAST_LOCAL);
	// kore_websocket_send(c, WEBSOCKET_OP_TEXT, data, len);
}

static void
websocket_disconnect(struct connection *c)
{
	// kore_log(LOG_NOTICE, "%p: disconnecting", c);
	kore_websocket_broadcast(c, WEBSOCKET_OP_TEXT, "User left chat", 14, WEBSOCKET_BROADCAST_LOCAL);
}
