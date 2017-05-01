/*
 * Copyright (c) 2015 Pebble Technology
 */
/*
* Matt Sandler 2017 Unofficial TinyCards app for Pebble watches.
*/

#pragma once

#include <pebble.h>

struct TinyCardAppMainWindowViewModel;

typedef void (*TinyCardAppMainWindowViewModelFunc)(struct TinyCardAppMainWindowViewModel* model);

typedef struct {
  TinyCardAppMainWindowViewModelFunc announce_changed;
  struct {
    GColor top;
    GColor bottom;
    int32_t to_bottom_normalized;
  } bg_color;
  char *fact;
  struct {
    int16_t idx;
    int16_t num;
    char text[8];
  } pagination;
} TinyCardAppMainWindowViewModel;

//! calls model's .announce_changed or does nothing if NULL
void tinycard_app_main_window_view_model_announce_changed(TinyCardAppMainWindowViewModel *model);

typedef struct {
  char side1[25];
  char side2[40];
} TinyCardAppDataPoint;




void tinycard_app_view_model_fill_strings_and_pagination(TinyCardAppMainWindowViewModel *view_model, TinyCardAppDataPoint *data_point);
  
void tinycard_app_view_model_fill_all(TinyCardAppMainWindowViewModel *model, TinyCardAppDataPoint *data_point);

void tinycard_app_view_model_fill_colors(TinyCardAppMainWindowViewModel *model, GColor color);

void tinycard_app_view_model_deinit(TinyCardAppMainWindowViewModel *model);

GColor tinycard_app_data_point_color(TinyCardAppDataPoint *data_point);

int tinycard_app_num_data_points(void);

TinyCardAppDataPoint *tinycard_app_data_point_at(int idx);
TinyCardAppDataPoint *tinycard_app_data_point_delta(TinyCardAppDataPoint *dp, int delta);
