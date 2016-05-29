#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_lower_text, *s_upper_text, *s_date_layer, *s_day_layer;
static Layer *s_background_layer;
static BitmapLayer *s_background_image_layer;
static GBitmap *s_background_image_bitmap;
static GFont s_time_font, s_date_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_hour_buffer[3];
  static char s_minute_buffer[3];
  static char date_buffer[8];
  static char day_buffer[4];
  
  strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ?
                                          "%H" : "%I", tick_time);
  strftime(s_minute_buffer, sizeof(s_minute_buffer),"%M", tick_time);
  strftime(date_buffer, sizeof(date_buffer), "%d %b", tick_time);
  strftime(day_buffer, sizeof(day_buffer), "%a", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_upper_text, s_hour_buffer);
  text_layer_set_text(s_lower_text, s_minute_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
  text_layer_set_text(s_day_layer, day_buffer);
}

static void bluetooth_callback(bool connected){  
  if(!connected){
    vibes_double_pulse();
  }
  
}

static void background_proc(Layer *layer, GContext *ctx){
  GRect bounds = layer_get_bounds(layer);
  
  //graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(bounds.size.w/2, bounds.size.h/2), 55);
  
  //graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(bounds.size.w/2, bounds.size.h/2), 53);
    
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window){
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WREXHAM_42));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WREXHAM_12));
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_background_image_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  s_background_image_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_image_layer, s_background_image_bitmap);
  
  s_background_layer = layer_create(GRect(17, 30, 112, 112));
  
  s_upper_text = text_layer_create(GRect(30, 40, 90, 50));
  
  text_layer_set_background_color(s_upper_text, GColorClear);
  //text_layer_set_text_color(s_back_text, GColorBlack);
  text_layer_set_text_color(s_upper_text, GColorWhite);
  text_layer_set_text(s_upper_text, "08");
  text_layer_set_font(s_upper_text, s_time_font);
  text_layer_set_text_alignment(s_upper_text, GTextAlignmentCenter);
  
  s_lower_text = text_layer_create(GRect(30, 75, 90, 50));
  
  text_layer_set_background_color(s_lower_text, GColorClear);
  //text_layer_set_text_color(s_front_text, GColorBlack);
  text_layer_set_text_color(s_lower_text, GColorWhite);
  text_layer_set_text(s_lower_text, "51");
  text_layer_set_font(s_lower_text, s_time_font);
  text_layer_set_text_alignment(s_lower_text, GTextAlignmentCenter);
  
  s_date_layer = text_layer_create(GRect(30, 37, 90, 50));
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  //text_layer_set_text_color(s_front_text, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text(s_date_layer, "27 May");
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  s_day_layer = text_layer_create(GRect(30, 120, 90, 50));
  
  text_layer_set_background_color(s_day_layer, GColorClear);
  //text_layer_set_text_color(s_front_text, GColorBlack);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_text(s_day_layer, "Wed");
  text_layer_set_font(s_day_layer, s_date_font);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_image_layer));
  layer_add_child(window_layer, s_background_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_upper_text));
  layer_add_child(window_layer, text_layer_get_layer(s_lower_text));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer)); 
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
  
  layer_set_update_proc(s_background_layer, background_proc);
  
  layer_mark_dirty(s_background_layer);
  
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  //window_set_background_color(s_main_window, GColorBlack);
  
}

static void main_window_unload(Window *window){
  gbitmap_destroy(s_background_image_bitmap);
  bitmap_layer_destroy(s_background_image_layer);
  
  text_layer_destroy(s_upper_text);
  text_layer_destroy(s_lower_text);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  
}

static void init(){
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(s_main_window, true);
  
  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_callback
  });
  
}

static void deinit(){
  window_destroy(s_main_window);
}


int main(){
  init();
  app_event_loop();
  deinit();
}