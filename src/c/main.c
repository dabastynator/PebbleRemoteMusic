#include <pebble.h>
#define MAX_LENGTH 30
#define TIME_LENGTH 7

static ActionBarLayer *action_bar;
static Layer * s_layer;
static Window *s_main_window;

char g_play_title[MAX_LENGTH];
char g_play_artist[MAX_LENGTH];
char g_time[TIME_LENGTH];

static GBitmap *s_icon_vol_up;
static GBitmap *s_icon_vol_down;
static GBitmap *s_icon_play;

const uint32_t inbox_size = 64;
const uint32_t outbox_size = 256;

static int VOL_UP = 1; 
static int VOL_DOWN = 0;
static int PLAY = 2;
static int DUMMY = 3;

static void update_layer_callback(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_frame(layer);

#ifdef PBL_ROUND
  GTextAttributes *attributes = graphics_text_attributes_create();
  graphics_text_attributes_enable_screen_text_flow(attributes, 8);
#endif
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(g_time, TIME_LENGTH, "%H:%M", t);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, g_time, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), 
                     GRect(5, 3, bounds.size.w - 40, 50), GTextOverflowModeWordWrap, 
                     GTextAlignmentCenter, PBL_IF_RECT_ELSE(NULL, attributes));
  graphics_draw_text(ctx, g_play_artist, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), 
                     GRect(5, 15, bounds.size.w - 40, 50), GTextOverflowModeWordWrap, 
                     GTextAlignmentLeft, PBL_IF_RECT_ELSE(NULL, attributes));
  graphics_draw_text(ctx, g_play_title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), 
                     GRect(5, 77, bounds.size.w - 40, 50), GTextOverflowModeWordWrap, 
                     GTextAlignmentLeft, PBL_IF_RECT_ELSE(NULL, attributes));

#ifdef PBL_ROUND
  graphics_text_attributes_destroy(attributes);
#endif
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Receive handler");
  Tuple *js_tuple = dict_find(iter, MESSAGE_KEY_TITLE);
  if(js_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Receive title");
    char *title = js_tuple->value->cstring;
    snprintf(g_play_title, MAX_LENGTH, "%s", title);
  }
  js_tuple = dict_find(iter, MESSAGE_KEY_ARTIST);
  if(js_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Receive artist");
    char *artist = js_tuple->value->cstring;
    snprintf(g_play_artist, MAX_LENGTH, "%s", artist);
  }
  layer_mark_dirty(s_layer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // A message was received, but had to be dropped
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // The message just sent has been successfully delivered
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Successfully send message to phone!");
}

static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  // The message just sent failed to be delivered
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
}

void click_vol_up(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if(result == APP_MSG_OK) {
    dict_write_int(iter, MESSAGE_KEY_VOLUME, &VOL_UP, 1, false);
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Start sending vol _up");
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

void click_vol_down(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if(result == APP_MSG_OK) {
    dict_write_int(iter, MESSAGE_KEY_VOLUME, &VOL_DOWN, 1, false);
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Start sending vol _down");
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

void click_play(ClickRecognizerRef recognizer, void *context) {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if(result == APP_MSG_OK) {
    dict_write_int(iter, MESSAGE_KEY_VOLUME, &PLAY, 1, false);
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Start sending play-pause");
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}


void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) click_vol_down);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) click_vol_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) click_play);
}

void main_window_load(Window *window) {
  s_icon_vol_up = gbitmap_create_with_resource(RESOURCE_ID_VOL_UP);
  s_icon_vol_down = gbitmap_create_with_resource(RESOURCE_ID_VOL_DOWN);
  s_icon_play = gbitmap_create_with_resource(RESOURCE_ID_PLAY);
  
  // Initialize the action bar:
  action_bar = action_bar_layer_create();
  // Associate the action bar with the window:
  action_bar_layer_add_to_window(action_bar, window);
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar,
                                             click_config_provider);

  // Set the icons:
  // The loading of the icons is omitted for brevity... See gbitmap_create_with_resource()
  action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, s_icon_vol_up, true);
  action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, s_icon_vol_down, true);
  action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_SELECT, s_icon_play, true);
  
  // Create textlayer for artist and song
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);  
  s_layer = layer_create(bounds);
  layer_set_update_proc(s_layer, update_layer_callback);
  layer_add_child(window_layer, s_layer);
  
  app_message_open(inbox_size, outbox_size);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);  
  
  snprintf(g_play_title, MAX_LENGTH, "%s", "");
  snprintf(g_play_artist, MAX_LENGTH, "%s", "No connection");
  
  // Send empty message to wakeup companion app
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if(result == APP_MSG_OK) {
    dict_write_int(iter, MESSAGE_KEY_VOLUME, &DUMMY, 1, false);
    result = app_message_outbox_send();
  }
}

static void main_window_unload(Window *window) {
  layer_destroy(s_layer);
}

static void init() {
  s_main_window = window_create();
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