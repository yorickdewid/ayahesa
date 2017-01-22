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

int						aya_init(int);
int						aya_connect(struct connection *);
static unsigned char *	aya_report(int code, char *title, size_t *length);

int						notfound(struct http_request *req);

/* Framework status page */
#if defined(STATUSPAGE)
int		status(struct http_request *);
#endif // STATUSPAGE

/* Optional framework routes */
#if defined(OPT_ROUTES)
int		shutdown_parent(struct http_request *);
int		fox(struct http_request *);
int		teapot(struct http_request *);
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

	return KORE_RESULT_OK;
}

int
aya_connect(struct connection *c)
{
	c->proto = CONN_PROTO_HTTP;

	net_recv_queue(c, http_header_max, NETBUF_CALL_CB_ALWAYS, http_header_recv);

	application_prelude();

	return KORE_RESULT_OK;
}

unsigned char *
aya_report(int code, char *title, size_t *length)
{
	static const char *default_report =
		"<html>"
		"<head>"
		"<title>Ayahesa Report</title>"
		"<style>"
		"table{font-family:arial,sans-serif;border-collapse:collapse;width:100%}"
		"th{background-color:#ed143d;color:#fff}"
		"td,th{border:1px solid #cccccc;text-align:left;padding:8px}"
		"tr:nth-child(even){background-color:#f1f1f1}"
		"</style>"
		"</head>"
		"<body>"
		"<h1>$title$</h1>"
		"<table>"
		"<tr><th colspan=\"2\">Ayahesa report details</th></tr>"
		"<tr><td>Code</td><td>$code$</td></tr>"
		"<tr><td>Message</td><td>$title$</td></tr>"
		"<tr><td>Occurrence</td><td>$time$</td></tr>"
		"<tr><td>Framework version</td><td>" VERSION "</td></tr>"
		"</table>"
		"</body>"
		"</html>";

	struct kore_buf		*buffer;
	char 				strcode[4];
	char 				*strtime;

	snprintf(strcode, 4, "%d", code);
	strtime = kore_time_to_date(time(NULL)),

	buffer = kore_buf_alloc(strlen(default_report));
	kore_buf_append(buffer, default_report, strlen(default_report));
	kore_buf_replace_string(buffer, "$title$", title, strlen(title));
	kore_buf_replace_string(buffer, "$code$", strcode, 3);
	kore_buf_replace_string(buffer, "$time$", strtime, strlen(strtime));
	return kore_buf_release(buffer, length);
}

int
notfound(struct http_request *req)
{
	size_t len;
	char *rep = (char *)aya_report(404, "Not Found", &len);

	http_response_header(req, "content-type", "text/html");
	http_response(req, 404, rep, len);
	kore_free(rep);

	return KORE_RESULT_OK;
}

#if defined(STATUSPAGE)

int
status(struct http_request *req)
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
		"<tr><th colspan=\"2\">Core</th></tr>"
		"<tr><td>Process</td><td>%d</td></tr>"
		"<tr><td>Instance</td><td>%s</td></tr>"
		"<tr><td>Domain</td><td>%s</td></tr>"
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
		size_t len;
		char *rep = (char *)aya_report(401, "Authorization required", &len);

		http_response_header(req, "content-type", "text/html");
		http_response(req, 404, rep, len);
		kore_free(rep);

		return KORE_RESULT_OK;
	}

	size_t default_page_length = strlen(default_page) + 512;
	char *buffer = (char *)kore_calloc(default_page_length, sizeof(char));
	snprintf(buffer, default_page_length,
		default_page,
		application_name(root_app),
		application_environment(root_app),
		application_session_lifetime(root_app),
		application_isdebug(root_app) ? "True" : "False",
		http_method_text(req->method),
		req->path,
		req->query_string,
		req->host,
		req->agent,
		http_remote_addr(req),
		getpid(),
		application_instance(),
		application_domainname(root_app),
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
	return KORE_RESULT_OK;
}

#endif // STATUSPAGE

#if defined(OPT_ROUTES)

int
shutdown_parent(struct http_request *req)
{
	size_t len;
	char *rep = (char *)aya_report(435, "External shutdown request", &len);

	kore_msg_send(KORE_MSG_PARENT, KORE_MSG_SHUTDOWN, "1", 1);
	http_response_header(req, "content-type", "text/html");
	http_response(req, 435, rep, len);
	kore_free(rep);
	return KORE_RESULT_OK;
}

int
fox(struct http_request *req)
{
	size_t len;
	char *rep = (char *)aya_report(419, "I'm a fox", &len);

	http_response_header(req, "content-type", "text/html");
	http_response(req, 419, rep, len);
	kore_free(rep);
	return KORE_RESULT_OK;
}

int
teapot(struct http_request *req)
{
	size_t len;
	char *rep = (char *)aya_report(418, "I'm a teapot", &len);

	http_response_header(req, "content-type", "text/html");
	http_response(req, 418, rep, len);
	kore_free(rep);
	return KORE_RESULT_OK;
}

#endif // OPT_ROUTES
