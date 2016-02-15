#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
static GFont s_time_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char buffer[] = "00:00:00";
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00:00"), "%H:%M:%S", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00:00"), "%I:%M:%S", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void sushi_step(){
  GRect bounds = layer_get_bounds(bitmap_layer_get_layer(s_background_layer));
  int16_t current_x = bounds.origin.x;
  
  // end = 720px - 144px
  if(current_x == -576) {
    layer_set_bounds(bitmap_layer_get_layer(s_background_layer), GRect(0, bounds.origin.y, bounds.size.w, bounds.size.h));
  }else{
    layer_set_bounds(bitmap_layer_get_layer(s_background_layer), GRect(current_x-24, bounds.origin.y, bounds.size.w, bounds.size.h));
  }
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}


static void main_window_load(Window *window) {
  //Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUSHI_S);
  s_background_layer = bitmap_layer_create(GRect(0, 60, 720, 72));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  //Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VIGRO_28));
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 18, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorDarkGreen);
  text_layer_set_text(s_time_layer, "00:00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  s_battery_layer = text_layer_create(GRect(0, 140, 144, 34));
  text_layer_set_text_color(s_battery_layer, GColorDarkGreen);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "100% charged");
  
  battery_state_service_subscribe(handle_battery);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  // Make sure the time is displayed from the start
  update_time();
  handle_battery(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_battery_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  sushi_step();
}
  
static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorPastelYellow);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
