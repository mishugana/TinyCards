/*
 * Copyright (c) 2015 Pebble Technology
 */
/*
* Matt Sandler 2017 Unofficial TinyCards app for Pebble watches.
*/
#include <pebble.h>
#include "tinycard_app_animations.h"
#include "tinycard_app_private.h"

static inline int16_t distance_interpolate(const int32_t normalized, int16_t from, int16_t to) {
  return from + ((normalized * (to - from)) / ANIMATION_NORMALIZED_MAX);
}


TinyCardAppMainWindowViewModel *view_model_from_animation(Animation *animation) {
  void *subject = NULL;
  property_animation_get_subject((PropertyAnimation *) animation, &subject);
  return subject;
}

static void update_bg_color_normalized(Animation *animation, const uint32_t distance_normalized) {
  TinyCardAppMainWindowViewModel *view_model = view_model_from_animation(animation);

  view_model->bg_color.to_bottom_normalized = distance_normalized;
  tinycard_app_main_window_view_model_announce_changed(view_model);
}

static const PropertyAnimationImplementation s_bg_color_normalized_implementation = {
  .base = {
    .update = (AnimationUpdateImplementation) update_bg_color_normalized,
  },
};

static void bg_colors_animation_started(Animation *animation, void *context) {
  TinyCardAppMainWindowViewModel *view_model = view_model_from_animation(animation);

  TinyCardAppDataPoint *dp = context;
  GColor color = tinycard_app_data_point_color(dp);

  // before, .top and .bottom are set to the current color, see weather_app_view_model_fill_colors()
  if (animation_get_reverse(animation)) {
    view_model->bg_color.top = color;
  } else {
    view_model->bg_color.bottom = color;
  }

  tinycard_app_main_window_view_model_announce_changed(view_model);
}

static void bg_colors_animation_stopped(Animation *animation, bool finished, void *context) {
  TinyCardAppMainWindowViewModel *view_model = view_model_from_animation(animation);

  TinyCardAppDataPoint *dp = context;
  GColor color = tinycard_app_data_point_color(dp);

  tinycard_app_view_model_fill_colors(view_model, color);
}

Animation *tinycard_app_create_view_model_animation_bgcolor(TinyCardAppMainWindowViewModel *view_model, TinyCardAppDataPoint *next_data_point) {
  Animation *bg_animation = (Animation *) property_animation_create(&s_bg_color_normalized_implementation, view_model, NULL, NULL);
  animation_set_handlers(bg_animation, (AnimationHandlers){
    .started = bg_colors_animation_started,
    .stopped = bg_colors_animation_stopped,
  }, next_data_point);
  return bg_animation;
}

