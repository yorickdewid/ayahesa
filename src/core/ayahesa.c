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

/* Application core operations */
extern void         application_create(app_t **);
extern void         application_config(app_t *, const char *);
extern void	        application_bootstrap(app_t *app);
extern void         application_release(app_t *);
extern void         application_prelude(struct connection *);
extern void         application_postproc(struct connection *c);

int             aya_init(int);
int             aya_connect(struct connection *);
void            aya_disconnect(struct connection *);

int             notfound(struct http_request *req);

#if defined(STATUSPAGE)
int		aya_status(struct http_request *);
#endif // STATUSPAGE

#if defined(OPT_ROUTES)
int     aya_readme(struct http_request *);
int     aya_shutdown_parent(struct http_request *);
int     aya_fox(struct http_request *);
int     aya_teapot(struct http_request *);
#endif // OPT_ROUTES

app_t *root_app = NULL;

int
aya_init(int state)
{
    switch (state) {
        case KORE_MODULE_LOAD:
            kore_log(LOG_NOTICE, "server core " VERSION);
            kore_log(LOG_NOTICE, "running Kore/%d.%d.%d-%s",
                KORE_VERSION_MAJOR,
                KORE_VERSION_MINOR,
                KORE_VERSION_PATCH,
                KORE_VERSION_STATE);
            kore_log(LOG_NOTICE, "load ayahesa");
            application_create(&root_app);
            application_config(root_app, CONFIG);
            application_bootstrap(root_app);
            http_server_version(VERSION);
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

#if defined(OPT_ROUTES)

int
aya_readme(struct http_request *request)
{
    FILE *fp = fopen("README.md", "r");
    if (!fp)
        return notfound(request);

    /* Determine file size */
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *string = kore_malloc(file_size + 1);
    fread(string, file_size, 1, fp);
    fclose(fp);

    http_response_header(request, "content-type", "text/plain");
    http_response(request, 200, string, file_size);
    kore_free(string);
    return_ok();
}

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
    http_view(request, 419, "report");
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
