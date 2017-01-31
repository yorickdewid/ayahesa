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
| Event Triggers
|--------------------------------------------------------------------------
|
| Here is where you can register event triggers for your application. These
| routes are called when a configuration URI is hit. Keep the routes
| simple and move any operations or actions into a controller or unit 
| method. Now create something great!
|
*/

#include <ayahesa.h>

/* Default log trigger */
trigger(auth_success)
{
    /* Subject is passed */
    const char *subject = (const char *)data;

    app_log("%s authenicated", subject);
}

struct aya_trigger trigger[] = {
    {EVENT_AUTH_SUCCESS, &trigger_auth_success},
    {0, NULL},
};
