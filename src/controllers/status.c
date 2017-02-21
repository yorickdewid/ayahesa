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

view_method(status, get_framework_version)
{
    return VERSION;
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
    if (http_auth_principal(request))
        status_has_auth_user = 1;

    http_response_header(request, "cache-control", "no-cache,no-store,must-revalidate");
    http_response_header(request, "pragma", "no-cache");
    http_response_header(request, "expires", "0");
    http_view(request, 200, "status");
    return_ok();
}
