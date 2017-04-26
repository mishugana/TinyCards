/*
 * Copyright (c) 2015 Pebble Technology
 */
/*
* Matt Sandler 2017 Unofficial TinyCards app for Pebble watches.
*/

#include <pebble.h>
#include "tinycard_app_private.h"
#include "tinycard_app_animations.h"
#include "tinycard_app_data.h"
#include "gdraw_command_transforms.h"

#define STATUS_BAR_HEIGHT 16

static Window *s_main_window;

static const int16_t MARGIN = 8;
static const int16_t ICON_DIMENSIONS = 48;

////////////////////
// update procs for our three custom layers

static void bg_update_proc(Layer *layer, GContext *ctx) {
  TinyCardAppData *data = window_get_user_data(s_main_window);
  TinyCardAppMainWindowViewModel *model = &data->view_model;
  const GRect bounds = layer_get_bounds(layer);

  int16_t y = (model->bg_color.to_bottom_normalized * bounds.size.h) / ANIMATION_NORMALIZED_MAX;

  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(model->bg_color.bottom, GColorWhite));
  GRect rect_top = bounds;
  rect_top.size.h = y;
  graphics_fill_rect(ctx, rect_top, 0, GCornerNone);

  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(model->bg_color.top, GColorWhite));
  GRect rect_bottom = bounds;
  rect_bottom.origin.y += y;
  graphics_fill_rect(ctx, rect_bottom, 0, GCornerNone);
}

static void horizontal_ruler_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  // y relative to layer's bounds to support clipping after some vertical scrolling
  const int16_t yy = 11;

  graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  graphics_draw_line(ctx, GPoint(0, yy), GPoint(bounds.size.w, yy));
}


////////////////////
// App boilerplate

//! helper to construct the various text layers as they appear in this app
static GRect init_text_layer(Layer *parent_layer, TextLayer **text_layer, int16_t y, int16_t h, int16_t additional_right_margin, char *font_key) {
  // why "-1" (and then "+2")? because for this font we need to compensate for weird white-spacing
  const int16_t font_compensator = strcmp(font_key, FONT_KEY_LECO_38_BOLD_NUMBERS) == 0 ? 3 : 1;

  const GRect frame = GRect(MARGIN - font_compensator, y, layer_get_bounds(parent_layer).size.w - 2 * MARGIN + 2 * font_compensator - additional_right_margin, h);

  *text_layer = text_layer_create(frame);
  text_layer_set_background_color(*text_layer, GColorClear);
  text_layer_set_text_color(*text_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  text_layer_set_font(*text_layer, fonts_get_system_font(font_key));
  layer_add_child(parent_layer, text_layer_get_layer(*text_layer));
  return frame;
}

void init_statusbar_text_layer(Layer *parent, TextLayer **layer) {
  init_text_layer(parent, layer, 0, STATUS_BAR_HEIGHT, 0, FONT_KEY_GOTHIC_14);
  GRect sb_bounds = layer_get_bounds(text_layer_get_layer(*layer));
  sb_bounds.origin.y -= 1;
  layer_set_bounds(text_layer_get_layer(*layer), sb_bounds);
  text_layer_set_text_alignment(*layer, GTextAlignmentCenter);
}

//! sets the new data model
static void set_data_point(TinyCardAppData *data, TinyCardAppDataPoint *dp) {
  data->data_point = dp;
  tinycard_app_view_model_fill_all(&data->view_model, dp);
}

static void view_model_changed(struct TinyCardAppMainWindowViewModel *arg) {
  TinyCardAppMainWindowViewModel *model = (TinyCardAppMainWindowViewModel *)arg;

  TinyCardAppData *data = window_get_user_data(s_main_window);

  text_layer_set_text(data->fact_layer, model->fact);
  text_layer_set_text(data->pagination_layer, model->pagination.text);

  // make sure to redraw (if no string pointer changed none of the layers would be dirty)
  layer_mark_dirty(window_get_root_layer(s_main_window));
}

static void main_window_load(Window *window) {
  TinyCardAppData *data = window_get_user_data(window);
  data->view_model.announce_changed = view_model_changed;

  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);
  layer_set_update_proc(window_layer, bg_update_proc);

  data->horizontal_ruler_layer = layer_create(GRect(MARGIN, 75, bounds.size.w - 2 * MARGIN, 55));
  layer_set_update_proc(data->horizontal_ruler_layer, horizontal_ruler_update_proc);
  layer_add_child(window_layer, data->horizontal_ruler_layer);

  init_text_layer(window_layer, &data->fact_layer, 60, 30, 0, FONT_KEY_GOTHIC_24_BOLD);
  text_layer_set_text_alignment(data->fact_layer, GTextAlignmentCenter);
  
  init_statusbar_text_layer(window_layer, &data->fake_statusbar);
  text_layer_set_text(data->fake_statusbar, "6:13 AM");
  //data->view_model.pagination.idx = 10;
  init_statusbar_text_layer(window_layer, &data->pagination_layer);
  text_layer_set_text_alignment(data->pagination_layer, GTextAlignmentRight);

  // propagate all view model content to the UI
  tinycard_app_main_window_view_model_announce_changed(&data->view_model);
}

static void main_window_unload(Window *window) {
  TinyCardAppData *data = window_get_user_data(window);
  data->view_model.announce_changed = NULL;
  tinycard_app_view_model_deinit(&data->view_model);

  layer_destroy(data->horizontal_ruler_layer);
  text_layer_destroy(data->fact_layer);
  text_layer_destroy(data->fake_statusbar);
  text_layer_destroy(data->pagination_layer);
}

static void after_scroll_swap_text(Animation *animation, bool finished, void *context) {
  TinyCardAppData *data = window_get_user_data(s_main_window);
  TinyCardAppDataPoint *data_point = context;

  tinycard_app_view_model_fill_strings_and_pagination(&data->view_model, data_point);
}

static Animation *create_anim_scroll_out(Layer *layer, uint32_t duration, int16_t dy) {
  GPoint to_origin = GPoint(0, dy);
  Animation *result = (Animation *) property_animation_create_bounds_origin(layer, NULL, &to_origin);
  animation_set_duration(result, duration);
  animation_set_curve(result, AnimationCurveLinear);
  return result;
}

static Animation *create_anim_scroll_in(Layer *layer, uint32_t duration, int16_t dy) {
  GPoint from_origin = GPoint(0, dy);
  Animation *result = (Animation *) property_animation_create_bounds_origin(layer, &from_origin, &GPointZero);
  animation_set_duration(result, duration);
  animation_set_curve(result, AnimationCurveEaseOut);
  return result;
}

static const uint32_t BACKGROUND_SCROLL_DURATION = 100 * 2;
static const uint32_t SCROLL_DURATION = 130 * 2;
static const int16_t SCROLL_DIST_OUT = 20;
static const int16_t SCROLL_DIST_IN = 8;

typedef enum {
  ScrollDirectionDown,
  ScrollDirectionRight,
  ScrollDirectionUp,
} ScrollDirection;

static Animation *create_outbound_anim(TinyCardAppData *data, ScrollDirection direction) {
  const int16_t to_dy = (direction == ScrollDirectionDown) ? -SCROLL_DIST_OUT : SCROLL_DIST_OUT;

  Animation *out_fact = create_anim_scroll_out(text_layer_get_layer(data->fact_layer), SCROLL_DURATION, to_dy);
  Animation *out_ruler = create_anim_scroll_out(data->horizontal_ruler_layer, SCROLL_DURATION, to_dy);

  return animation_spawn_create(out_fact, out_ruler, NULL);
}

static Animation *create_inbound_anim(TinyCardAppData *data, ScrollDirection direction) {
  const int16_t from_dy = (direction == ScrollDirectionDown) ? -SCROLL_DIST_IN : SCROLL_DIST_IN;

  Animation *in_fact = create_anim_scroll_in(text_layer_get_layer(data->fact_layer), SCROLL_DURATION, from_dy);
  Animation *in_ruler = create_anim_scroll_in(data->horizontal_ruler_layer, SCROLL_DURATION, from_dy);

  return animation_spawn_create(in_fact, in_ruler, NULL);
}

static Animation *animation_for_scroll(TinyCardAppData *data, ScrollDirection direction, TinyCardAppDataPoint *next_data_point) {
  TinyCardAppMainWindowViewModel *view_model = &data->view_model;

  // sliding texts
  Animation *out_text = create_outbound_anim(data, direction);
  animation_set_handlers(out_text, (AnimationHandlers) {
    .stopped = after_scroll_swap_text,
  }, next_data_point);
  Animation *in_text = create_inbound_anim(data, direction);

  // scrolling background color
  Animation *bg_animation = tinycard_app_create_view_model_animation_bgcolor(view_model, next_data_point);
  animation_set_duration(bg_animation, BACKGROUND_SCROLL_DURATION);
  animation_set_reverse(bg_animation, (direction == ScrollDirectionDown));


  return animation_spawn_create(animation_sequence_create(out_text, in_text, NULL), bg_animation,   NULL);
}

static Animation *animation_for_bounce(TinyCardAppData *data, ScrollDirection direction) {
  return create_inbound_anim(data, direction);
}

static void ask_for_scroll(TinyCardAppData *data, ScrollDirection direction) {
  if (direction != ScrollDirectionRight){
      int delta = direction == ScrollDirectionUp ? -1 : +1;
      TinyCardAppDataPoint *next_data_point = tinycard_app_data_point_delta(data->data_point, delta);
    
      Animation *scroll_animation;
    
      if (!next_data_point) {
        scroll_animation = animation_for_bounce(data, direction);
      } else {
        // data point switches immediately
        data->data_point = next_data_point;
        scroll_animation = animation_for_scroll(data, direction, next_data_point);
      }
    
      animation_unschedule(data->previous_animation);
      animation_schedule(scroll_animation);
      data->previous_animation = scroll_animation;
    }
  else
  {
      text_layer_set_text(data->fact_layer, data->data_point->side2);
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  TinyCardAppData *data = context;
  ask_for_scroll(data, ScrollDirectionUp);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  TinyCardAppData *data = context;
  //ask_for_scroll(data, ScrollDirectionRight);
  
  if(strcmp(text_layer_get_text(data->fact_layer) , data->data_point->side1))
    text_layer_set_text(data->fact_layer, data->data_point->side1);
  else
    text_layer_set_text(data->fact_layer, data->data_point->side2);
  
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  TinyCardAppData *data = context;
  ask_for_scroll(data, ScrollDirectionDown);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *side1_tuple = dict_find(iterator,  MESSAGE_KEY_SIDE1);
  Tuple *side2_tuple = dict_find(iterator,  MESSAGE_KEY_SIDE2);
  Tuple *number_tuple = dict_find(iterator,  MESSAGE_KEY_NUMBER);
   APP_LOG(APP_LOG_LEVEL_INFO, "Number %d, Side 1 %s, Side 2 %s",(int)number_tuple->value->int32 , side1_tuple->value->cstring, side2_tuple->value->cstring);
  strncpy(tinycard_app_data_point_at((int)number_tuple->value->int32)->side1 , side1_tuple->value->cstring,20);
  strncpy(tinycard_app_data_point_at((int)number_tuple->value->int32)->side2 , side2_tuple->value->cstring,20);
  TinyCardAppData *data = window_get_user_data(s_main_window);
  data->view_model.announce_changed = view_model_changed;

}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  TinyCardAppData *data = malloc(sizeof(TinyCardAppData));
  memset(data, 0, sizeof(TinyCardAppData));
  TinyCardAppDataPoint *dp = tinycard_app_data_point_at(0);
  set_data_point(data, dp);
  s_main_window = window_create();
  
    // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  //Open Appmessage
  const int inbox_size = 2024;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  window_set_click_config_provider_with_context(s_main_window, click_config_provider, data);
  window_set_user_data(s_main_window, data);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  

}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
