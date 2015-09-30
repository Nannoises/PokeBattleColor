#include <pebble.h>

Window *window;
static GBitmap *s_bitmap = NULL;
static GBitmap *e_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static BitmapLayer *e_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;
static GBitmapSequence *e_sequence = NULL;

static GFont *time_font;
static GFont *date_font;
TextLayer *text_time_layer;
TextLayer *text_date_layer;

static GBitmap *icon_battery_charge;
static uint8_t battery_level;
static bool battery_plugged;
static Layer *battery_layer;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static uint8_t hour_progression;
static Layer *hour_progression_layer;

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

    // Timer for that delay
    app_timer_register(next_delay, timer_handler, NULL);
  } else {
    // Start again
    gbitmap_sequence_restart(s_sequence);
  }
}

static void e_timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(e_sequence, e_bitmap, &next_delay)) {
    bitmap_layer_set_bitmap(e_bitmap_layer, e_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(e_bitmap_layer));

    // Timer for that delay
    app_timer_register(next_delay, e_timer_handler, NULL);
  } else {
    // Start again
    gbitmap_sequence_restart(e_sequence);
  }
}

static void load_sequence() {
  // Free old data
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  // Create 
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_CHARIZARD_BACK);

  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Begin animation
  app_timer_register(1, timer_handler, NULL);
}

static void load_e_sequence() {   
  // Free old data
  if(e_sequence) {
    gbitmap_sequence_destroy(e_sequence);
    e_sequence = NULL;
  }
  if(e_bitmap) {
    gbitmap_destroy(e_bitmap);
    e_bitmap = NULL;
  }

  // Create 
  e_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_BLASTOISE);

  // Create GBitmap
  e_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(e_sequence), GBitmapFormat8Bit);

  // Begin animation
  app_timer_register(1, e_timer_handler, NULL);
}

static void load_time_text_layer(Layer *window_layer)
{
  text_time_layer = text_layer_create(GRect(12, 132, 124, 30));
 	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
 	text_layer_set_text_color(text_time_layer, GColorBlack);
 	text_layer_set_background_color(text_time_layer, GColorClear);
 	text_layer_set_font(text_time_layer, time_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));  
}

static void load_date_text_layer(Layer *window_layer)
{
  text_date_layer = text_layer_create(GRect(49, 106, 76, 10));	
	text_layer_set_text_alignment(text_date_layer, GTextAlignmentRight);
 	text_layer_set_text_color(text_date_layer, GColorBlack);
 	text_layer_set_background_color(text_date_layer, GColorClear);
 	text_layer_set_font(text_date_layer, date_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
}

static void load_ally_pokemon_layer(Layer *window_layer)
{
  GRect bounds = layer_get_bounds(window_layer);  
  bounds.origin.x -= 50;
  bounds.origin.y += 15;

  s_bitmap_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  load_sequence();
}

static void load_enemy_pokemon_layer(Layer *window_layer)
{
  GRect e_bounds = layer_get_bounds(window_layer);
  e_bounds.origin.x += 35;
  e_bounds.origin.y -= 50;  
  
  e_bitmap_layer = bitmap_layer_create(e_bounds);
  bitmap_layer_set_compositing_mode(e_bitmap_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(e_bitmap_layer));
  
  load_e_sequence();
}

static void load_background_layer(Layer *window_layer)
{
  background_image = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_compositing_mode(background_layer, GCompOpSet);
  
  bitmap_layer_set_bitmap(background_layer, background_image);
 	layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
}

static GColor8 get_color_by_percent(uint8_t percent)
{
    GColor8 color = GColorBlack;
    if(battery_level > 50)
    {
       color = GColorMalachite;      
    }
    else if(battery_level > 25)
    {
      color = GColorYellow;  
    }
    else
    {
      color = GColorRed;
    }
    return color;
}

void battery_layer_update_callback(Layer *layer, GContext *ctx) {

  	graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    
    GColor8 batteryColor = get_color_by_percent(battery_level);
  
  	graphics_context_set_stroke_color(ctx, batteryColor);
  	graphics_context_set_fill_color(ctx,  batteryColor);

  	if (!battery_plugged) {
    	graphics_fill_rect(ctx, GRect(0, 0, (uint8_t)(battery_level)/2, 3), 0, GCornerNone);
 	}
  	else {	
    	graphics_draw_bitmap_in_rect(ctx, icon_battery_charge, GRect(1, 0, 50, 2));
 	}
}

static void load_battery_layer(Layer *window_layer)
{
  icon_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);
 	BatteryChargeState initial = battery_state_service_peek();
 	battery_level = initial.charge_percent;
 	battery_plugged = initial.is_plugged;
 	battery_layer = layer_create(GRect(88,100,50,3));
 	layer_set_update_proc(battery_layer, &battery_layer_update_callback);  
  layer_add_child(window_layer, battery_layer);
}

void hour_progression_layer_update_callback(Layer *layer, GContext *ctx)
{
  GColor8 hourColor = get_color_by_percent(hour_progression);
 	graphics_context_set_stroke_color(ctx, hourColor);
 	graphics_context_set_fill_color(ctx,  hourColor);
  graphics_fill_rect(ctx, GRect(0, 0, (uint8_t)(hour_progression)/2, 3), 0, GCornerNone);
}

static void load_hour_progression_layer(Layer *window_layer)
{
  hour_progression_layer = layer_create(GRect(21, 31, 50, 3));
  layer_set_update_proc(hour_progression_layer, &hour_progression_layer_update_callback);
  layer_add_child(window_layer, hour_progression_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  load_ally_pokemon_layer(window_layer);
  
  load_enemy_pokemon_layer(window_layer);
    
  load_background_layer(window_layer);
  
  load_battery_layer(window_layer);
  
  load_hour_progression_layer(window_layer);
  
  load_time_text_layer(window_layer);
  
  load_date_text_layer(window_layer);
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{    
 	static char time_text[] = "00:00";
 	static char date_text[] = "Xxx,00.00.";

 	char *time_format;
 	char *date_format;
  
	time_format = "%I:%M";	// 0:00
	date_format = "%b%e";	// Dec31	
  
  strftime(time_text, sizeof(time_text), time_format, tick_time);
 	strftime(date_text, sizeof(date_text), date_format, tick_time);  
  if (time_text[0] == '0') {
   		memmove(time_text, &time_text[1], sizeof(time_text) - 1);
	}
  
  hour_progression = ((1 - (double)tick_time->tm_min / 60)) * 100;
  layer_mark_dirty(hour_progression_layer);
  
  text_layer_set_text(text_time_layer, time_text);
 	text_layer_set_text(text_date_layer, date_text);
}

void battery_state_handler(BatteryChargeState charge) {
	battery_level = charge.charge_percent;
  battery_plugged = charge.is_plugged;
  layer_mark_dirty(battery_layer);
}

void handle_init(void) {
  time_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_24));
  date_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_7));
  
	// Create a window 
  window = window_create();
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

	// Push the window
	window_stack_push(window, true);	
  
  time_t now = time(NULL);
 	struct tm *tick_time = localtime(&now);
	handle_minute_tick(tick_time, MINUTE_UNIT);
 	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);  
  
  battery_state_service_subscribe (&battery_state_handler);
  
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
	// TODO destroy stuff
  
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
