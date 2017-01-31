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
| Here is where you can register web routes for your application. These
| routes are called when a configuration URI is hit. Keep the routes
| simple and move any operations or actions into a controller or unit 
| method. Now create something great!
|
*/

#include <ayahesa.h>

static void
trigger_log(void *data)
{
    app_log("Object authenicated");
}

struct aya_trigger trigger[] = {
    {EVENT_AUTH_SUCCESS, &trigger_log},
    {0, NULL},
};
