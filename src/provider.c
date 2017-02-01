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
| Application Provider
|--------------------------------------------------------------------------
|
| Here is where you can register event triggers for your application.
| Whenever a framework event is fired you can specify triggers to act on
| the action. Per trigger data can be passed in. See the documentation for
| the correct structures.
|
*/

#include <ayahesa.h>

struct aya_providers providers[] = {
    {"quid", NULL, NULL},
    {"", NULL, NULL},
};
