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

int					aya_init(int);
int					aya_connect(struct connection *);
void				aya_disconnect(struct connection *);

int					notfound(struct http_request *req);

#if defined(STATUSPAGE)
int		aya_status(struct http_request *);
#endif // STATUSPAGE

#if defined(OPT_ROUTES)
int		aya_shutdown_parent(struct http_request *);
int		aya_fox(struct http_request *);
int		aya_teapot(struct http_request *);
#endif // OPT_ROUTES

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

	return_ok();
}

int
aya_connect(struct connection *c)
{
	c->proto = CONN_PROTO_HTTP;

	net_recv_queue(c, http_header_max, NETBUF_CALL_CB_ALWAYS, http_header_recv);

	c->disconnect = aya_disconnect;

	/* Bootstrap connection */
	application_prelude(c);

	return_ok();
}

void
aya_disconnect(struct connection *c)
{
	/* Process cleanup */
	application_postproc(c);
}

int
notfound(struct http_request *request)
{
	size_t len;
	char *report = http_report(404, "Not Found", &len);

	http_response_header(request, "content-type", "text/html");
	http_response(request, 404, report, len);
	kore_free(report);
	return_ok();
}

#if defined(STATUSPAGE)

int
aya_status(struct http_request *request)
{
	const char *default_page =
		"<html>"
		"<head>"
		"<title>Ayahesa Core</title>"
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
		"<tr><td>Session lifetime</td><td>%d</td></tr>"
		"<tr><td>Debug</td><td>%s</td></tr>"
		"<tr><th colspan=\"2\">Environment</th></tr>"
		"<tr><td>Method</td><td>%s</td></tr>"
		"<tr><td>Path</td><td>%s</td></tr>"
		"<tr><td>Query string</td><td>%s</td></tr>"
		"<tr><td>Host</td><td>%s</td></tr>"
		"<tr><td>Agent</td><td>%s</td></tr>"
		"<tr><td>Remote</td><td>%s</td></tr>"
		"<tr><td>User</td><td>%s</td></tr>"
		"<tr><th colspan=\"2\">Core</th></tr>"
		"<tr><td>Process</td><td>%d</td></tr>"
		"<tr><td>Instance</td><td>%s</td></tr>"
		"<tr><td>Domain</td><td>%s</td></tr>"
		"<tr><td>Uptime</td><td>%s</td></tr>"
		"<tr><td>Requests</td><td>%d</td></tr>"
		"<tr><td>Active connections</td><td>%d</td></tr>"
		"<tr><td>Servertime</td><td>%s</td></tr>"
		"<tr><td>Framework version</td><td>" VERSION "</td></tr>"
		"<tr><td>Servlet version</td><td>" SERVLET_VERSION "</td></tr>"
		"</table>"
		"</body>"
		"</html>";

	/* Only GET */
	http_get();

	/* Fill HTML structure */
	size_t default_page_length = strlen(default_page) + 512;
	char *buffer = (char *)kore_calloc(default_page_length, sizeof(char));
	snprintf(buffer, default_page_length,
		default_page,
		app_name(),
		app_environment(),
		app_session_lifetime(),
		app_isdebug() ? "True" : "False",
		http_method_text(request->method),
		request->path,
		request->query_string,
		request->host,
		request->agent,
		http_remote_addr(request),
		request->hdlr_extra ? ((struct request_data *)request->hdlr_extra)->auth.principal : "(null)",
		getpid(),
		app_instance(),
		app_domainname(),
		app_uptime(),
		app_request_count(),
		app_active_conncount(),
		kore_time_to_date(time(NULL)));

	http_response_header(request, "content-type", "text/html");
	http_response(request, 200, buffer, strlen(buffer));
	kore_free(buffer);
	return_ok();
}

#endif // STATUSPAGE

#if defined(OPT_ROUTES)

int
aya_shutdown_parent(struct http_request *request)
{
	size_t len;
	char *report = http_report(435, "External shutdown request", &len);

	kore_msg_send(KORE_MSG_PARENT, KORE_MSG_SHUTDOWN, "1", 1);
	http_response_header(request, "content-type", "text/html");
	http_response(request, 435, report, len);
	kore_free(report);
	return_ok();
}

int
aya_fox(struct http_request *request)
{
	size_t len;
	char *report = http_report(419, "I'm a fox", &len);

	http_response_header(request, "content-type", "text/html");
	http_response(request, 419, report, len);
	kore_free(report);
	return_ok();
}

int
aya_teapot(struct http_request *request)
{
	size_t len;
	char *report = http_report(418, "I'm a teapot", &len);

	http_response_header(request, "content-type", "text/html");
	http_response(request, 418, report, len);
	kore_free(report);
	return_ok();
}

#endif // OPT_ROUTES
