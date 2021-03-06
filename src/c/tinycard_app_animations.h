/*
 * Copyright (c) 2015 Pebble Technology
 */
/*
* Matt Sandler 2017 Unofficial TinyCards app for Pebble watches.
*/
#pragma once

#include <pebble.h>
#include "tinycard_app_data.h"


Animation *tinycard_app_create_view_model_animation_bgcolor(TinyCardAppMainWindowViewModel *view_model, TinyCardAppDataPoint *next_data_point);

