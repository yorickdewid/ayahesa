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

#include "assets.h"

int status_has_auth_user = 0;

view_method(status, get_session_lifetime)
{
    return aya_itoa(app_session_lifetime());
}

view_method(status, get_debug)
{
    return app_isdebug() ? "True" : "False";
}

view_method(status, get_storage)
{
    return app_storage();
}

view_method(status, get_pid)
{
    return aya_itoa(getpid());
}

view_method(status, get_requests)
{
    return aya_itoa(app_request_count());
}

view_method(status, get_active_conn)
{
    return aya_itoa(app_active_conncount());
}

view_method(status, get_kore_version)
{
    static char kversion[32];
    snprintf(kversion, 32, "Kore/%d.%d.%d-%s",
        KORE_VERSION_MAJOR,
        KORE_VERSION_MINOR,
        KORE_VERSION_PATCH,
        KORE_VERSION_STATE);
    return kversion;
}

/**
 * Status controller
 *
 * Retrieve status page. See the
 * static asset for the HTML view.
 * Headers are set to prevent client
 * side caching.
 */
controller(get_status)
{
    if (!request->hdlr_extra)
        request->hdlr_extra = kore_calloc(1, sizeof(struct request_data));

    if (http_auth_principal(request))
        status_has_auth_user = 1;

    /* Setup session tree */
    struct request_data *session = (struct request_data *)request->hdlr_extra;
    session->session = (app_t *)kore_malloc(sizeof(app_t));
    session->session->type = T_NULL;
    session->session->flags = 0;
    session->session->key = NULL;
    tree_new_root(session->session);

    /* Push view variables */
    tree_put_str(session->session, "background-clr", "#9370db");
    tree_put_str(session->session, "title", "Framework Status Page");

    http_response_header(request, "cache-control", "no-cache,no-store,must-revalidate");
    http_response_header(request, "pragma", "no-cache");
    http_response_header(request, "expires", "0");
    http_view(request, 200, "status");

    /* Release session tree */
    tree_free(session->session);
    kore_free(request->hdlr_extra);
    request->hdlr_extra = NULL;

    return_ok();
}
