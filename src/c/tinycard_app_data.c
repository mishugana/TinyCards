/*
 * Copyright (c) 2015 Pebble Technology
 */
/*
* Matt Sandler 2017 Unofficial TinyCards app for Pebble watches.
*/

#include <pebble.h>
#include "tinycard_app_data.h"

void tinycard_app_main_window_view_model_announce_changed(TinyCardAppMainWindowViewModel *model) {
  if (model->announce_changed) {
    model->announce_changed((struct TinyCardAppMainWindowViewModel *)model);
  }
}

int tinycard_app_index_of_data_point(TinyCardAppDataPoint *dp);

void tinycard_app_view_model_fill_strings_and_pagination(TinyCardAppMainWindowViewModel *view_model, TinyCardAppDataPoint *data_point) {
  view_model->fact = data_point->side1;

  view_model->pagination.idx = (int16_t)(1 + tinycard_app_index_of_data_point(data_point));
  view_model->pagination.num = (int16_t)tinycard_app_num_data_points();
  snprintf(view_model->pagination.text, sizeof(view_model->pagination.text), "%d/%d", view_model->pagination.idx, view_model->pagination.num);
  tinycard_app_main_window_view_model_announce_changed(view_model);
}


void tinycard_app_view_model_fill_colors(TinyCardAppMainWindowViewModel *model, GColor color) {
  model->bg_color.top = color;
  model->bg_color.bottom = color;
  tinycard_app_main_window_view_model_announce_changed(model);
}

GColor tinycard_app_data_point_color(TinyCardAppDataPoint *dp) {
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "Page %d", tinycard_app_index_of_data_point(dp));
  return (tinycard_app_index_of_data_point(dp) % 2) ? GColorPictonBlue : GColorMayGreen;
}

void tinycard_app_view_model_fill_all(TinyCardAppMainWindowViewModel *model, TinyCardAppDataPoint *data_point) {
  TinyCardAppMainWindowViewModelFunc annouce_changed = model->announce_changed;
  memset(model, 0, sizeof(*model));
  model->announce_changed = annouce_changed;
  tinycard_app_view_model_fill_strings_and_pagination(model, data_point);
  tinycard_app_view_model_fill_colors(model, tinycard_app_data_point_color(data_point));

  tinycard_app_main_window_view_model_announce_changed(model);
}

void tinycard_app_view_model_deinit(TinyCardAppMainWindowViewModel *model) {

}

static TinyCardAppDataPoint s_data_points[] = {
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
        {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },
    {
        .side1 = "Hola",
        .side2 = "Hello",
    },

};



int tinycard_app_num_data_points(void) {
  return ARRAY_LENGTH(s_data_points);
}

TinyCardAppDataPoint *tinycard_app_data_point_at(int idx) {
  if (idx < 0 || idx > tinycard_app_num_data_points() - 1) {
    return NULL;
  }

  return &s_data_points[idx];
}

int tinycard_app_index_of_data_point(TinyCardAppDataPoint *dp) {
  for (int i = 0; i < tinycard_app_num_data_points(); i++) {
    if (dp == tinycard_app_data_point_at(i)) {
      return i;
    }
  }
  return -1;
}

TinyCardAppDataPoint *tinycard_app_data_point_delta(TinyCardAppDataPoint *dp, int delta) {
  int idx = tinycard_app_index_of_data_point(dp);
  if (idx < 0) {
    return NULL;
  }
  return tinycard_app_data_point_at(idx + delta);
}
