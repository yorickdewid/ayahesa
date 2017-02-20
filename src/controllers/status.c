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

char *status_get_session_lifetime(int argc, char *argv[]);
char *
status_get_session_lifetime(int argc, char *argv[])
{
    return aya_itoa(app_session_lifetime());
}

char *status_get_debug(int argc, char *argv[]);
char *
status_get_debug(int argc, char *argv[])
{
    return app_isdebug() ? "True" : "False";
}

char *status_get_storage(int argc, char *argv[]);
char *
status_get_storage(int argc, char *argv[])
{
    return app_storage();
}

char *status_get_pid(int argc, char *argv[]);
char *
status_get_pid(int argc, char *argv[])
{
    return aya_itoa(getpid());
}

char *status_get_requests(int argc, char *argv[]);
char *
status_get_requests(int argc, char *argv[])
{
    return aya_itoa(app_request_count());
}

char *status_get_active_conn(int argc, char *argv[]);
char *
status_get_active_conn(int argc, char *argv[])
{
    return aya_itoa(app_active_conncount());
}

char *status_get_framework_version(int argc, char *argv[]);
char *
status_get_framework_version(int argc, char *argv[])
{
    return VERSION;
}

char *status_get_kore_version(int argc, char *argv[]);
char *
status_get_kore_version(int argc, char *argv[])
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
 */
controller(get_status)
{
    if (http_auth_principal(request))
        status_has_auth_user = 1;

    return view(request, "status");
}
