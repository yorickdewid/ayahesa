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

#include <time.h>

#define SERVLET_VERSION		"Kytarah/0.3"

int		aya_init(int);
int		aya_connect(struct connection *);
void	aya_disconnect(struct connection *);

int		status(struct http_request *);
int		shutdown_parent(struct http_request *);
int		fox(struct http_request *);
int		teapot(struct http_request *);

app_t *root_app = NULL;

int
aya_init(int state)
{
	switch (state) {
		case KORE_MODULE_LOAD:
			kore_log(LOG_NOTICE, "server core " SERVLET_VERSION);
			kore_log(LOG_NOTICE, "load ayahesa");
			application_create(&root_app);
			application_config(root_app, CONFIG);
			http_server_version(SERVLET_VERSION);
			break;
		case KORE_MODULE_UNLOAD:
			kore_log(LOG_NOTICE, "unload ayahesa");
			application_release(root_app);
			break;
		default:
			kore_log(LOG_NOTICE, "state %d unknown", state);
			break;
	}

	return (KORE_RESULT_OK);
}

int
aya_connect(struct connection *c)
{
	c->proto = CONN_PROTO_HTTP;

	net_recv_queue(c, http_header_max, NETBUF_CALL_CB_ALWAYS, http_header_recv);

	// c->disconnect = aya_disconnect;

	application_prelude();

	return (KORE_RESULT_OK);
}

void
aya_disconnect(struct connection *c)
{
	
}

#if defined(STATUSPAGE)

int
status(struct http_request *req)
{
	const char *default_page =
		"<html>"
		"<head>"
		"<title>Ayahesa</title>"
		"<style>"
		"table{font-family:arial,sans-serif;border-collapse:collapse;width:100%%}"
		"th{background-color:#9370db;color:#fff}"
		"td,th{border:1px solid #cccccc;text-align:left;padding:8px}"
		"tr:nth-child(even){background-color:#f1f1f1}"
		"</style>"
		"</head>"
		"<body>"
		"<h1>Ayahesa Core</h1>"
		"<table>"
		"<tr><th colspan=\"2\">Application</th></tr>"
		"<tr><td>Application</td><td>%s</td></tr>"
		"<tr><td>Environment</td><td>%s</td></tr>"
		"<tr><td>Domain</td><td>%s</td></tr>"
		"<tr><td>Session lifetime</td><td>%d</td></tr>"
		"<tr><td>Debug</td><td>%s</td></tr>"
		"<tr><th colspan=\"2\">Environment</th></tr>"
		"<tr><td>Method</td><td>%s</td></tr>"
		"<tr><td>Path</td><td>%s</td></tr>"
		"<tr><td>Query string</td><td>%s</td></tr>"
		"<tr><td>Host</td><td>%s</td></tr>"
		"<tr><td>Agent</td><td>%s</td></tr>"
		"<tr><td>Remote</td><td>%s</td></tr>"
		"<tr><th colspan=\"2\">Core</th></tr>"
		"<tr><td>Instance</td><td>%s</td></tr>"
		"<tr><td>Uptime</td><td>%s</td></tr>"
		"<tr><td>Servertime</td><td>%s</td></tr>"
		"<tr><td>Requests</td><td>%d</td></tr>"
		"<tr><td>Framework version</td><td>" VERSION "</td></tr>"
		"<tr><td>Servlet version</td><td>" SERVLET_VERSION "</td></tr>"
		"</table>"
		"</body>"
		"</html>";

	/* Protect route with basic authentication */
	if (!http_basic_auth(req, STATUSPAGE_AUTH)) {
		http_response_header(req, "www-authenticate", "Basic realm=\"Status page\"");
		const char *error_page =
			"<html>"
			"<head>"
			"<title>Ayahesa</title>"
			"<style>"
			"table{font-family:arial,sans-serif;border-collapse:collapse;width:100%}"
			"th{background-color:#ed143d;color:#fff}"
			"td,th{border:1px solid #cccccc;text-align:left;padding:8px}"
			"tr:nth-child(even){background-color:#f1f1f1}"
			"</style>"
			"</head>"
			"<body>"
			"<h1>Authorization required</h1>"
			"<table>"
			"<tr><th colspan=\"2\">Details</th></tr>"
			"<tr><td>Error</td><td>401</td></tr>"
			"<tr><td>Message</td><td>Authorization required</td></tr>"
			"</table>"
			"</body>"
			"</html>";

		http_response(req, 401, error_page, strlen(error_page));
		return (KORE_RESULT_OK);
	}

	char *buffer = (char *)kore_malloc(strlen(default_page) + 256);
	memset(buffer, '\0', strlen(default_page) + 256);

	sprintf(buffer, default_page,
		application_name(root_app),
		application_environment(root_app),
		application_domainname(root_app),
		application_session_lifetime(root_app),
		application_isdebug(root_app) ? "True" : "False",
		http_method_text(req->method),
		req->path,
		req->query_string,
		req->host,
		req->agent,
		http_remote_addr(req),
		application_instance(),
		application_uptime(root_app),
		kore_time_to_date(time(NULL)),
		application_request_count());

	if (req->method != HTTP_METHOD_GET) {
		http_response_header(req, "allow", "get");
		http_response(req, 405, NULL, 0);
		return (KORE_RESULT_OK);
	}

	http_response_header(req, "content-type", "text/html");
	http_response(req, 200, buffer, strlen(buffer));
	kore_free(buffer);
	return (KORE_RESULT_OK);
}

#endif // STATUSPAGE

#if defined(OPT_ROUTES)

int
shutdown_parent(struct http_request *req)
{
    kore_msg_send(KORE_MSG_PARENT, KORE_MSG_SHUTDOWN, "1", 1);
	http_response(req, 435, NULL, 0);
	return (KORE_RESULT_OK);
}

int
fox(struct http_request *req)
{
	http_response_header(req, "content-type", "text/plain");
	http_response(req, 419, "I'm a fox.", 10);
	return (KORE_RESULT_OK);
}

int
teapot(struct http_request *req)
{
	http_response_header(req, "content-type", "text/plain");
	http_response(req, 418, "I'm a teapot.", 13);
	return (KORE_RESULT_OK);
}

#if 0
int middleware_session(struct http_request *req, char *data);
int
middleware_session(struct http_request *req, char *data)
{
	//kore_log(LOG_NOTICE, "middleware_session: %s", data);
	printf("middleware_session: %s\n", data);

	if (!strcmp(data, "ABC@123"))
		return (KORE_RESULT_OK);

	return (KORE_RESULT_ERROR);
}
#endif

#endif // OPT_ROUTES
