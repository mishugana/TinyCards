/*
 * Copyright (c) 2015 Pebble Technology
 */
/*
* Matt Sandler 2017 Unofficial TinyCards app for Pebble watches.
*/

#pragma once

#include <pebble.h>
#include "tinycard_app_data.h"

typedef struct {
  TinyCardAppDataPoint *data_point;
  TinyCardAppMainWindowViewModel view_model;
  Animation *previous_animation;
  TextLayer *fake_statusbar;
  TextLayer *pagination_layer;
  TextLayer *fact_layer;
  Layer *horizontal_ruler_layer;

} TinyCardAppData;
