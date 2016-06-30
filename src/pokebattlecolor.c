#include <pebble.h>

Window *window;
static GBitmap *s_bitmap = NULL;
static GBitmap *e_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static BitmapLayer *e_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;
static GBitmapSequence *e_sequence = NULL;

static bool animate = false;
static bool shinyAlly = false;

static GFont time_font;
static GFont date_font;
TextLayer *text_time_layer;
TextLayer *text_date_layer;

static GFont pokemon_name_font;
TextLayer *ally_pokemon_name_layer;
TextLayer *enemy_pokemon_name_layer;

static uint8_t battery_level;
static bool battery_plugged;
static Layer *battery_layer;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static uint8_t hour_progression;
static Layer *hour_progression_layer;

static GBitmap *ally_status_sleep_image;
static BitmapLayer *ally_status_sleep_layer;

static GBitmap *ally_status_par_image;
static BitmapLayer *ally_status_par_layer;

static bool initiate_watchface = true;

char *ALLY_POKEMON_NAME = "CHARIZARD    ";
char *ENEMY_POKEMON_NAME = "BLASTOISE   ";
  
static GFont level_font;
int level_int = 1;
int level_int_2 = 1;

static char level_string[10];
static char level_string_2[10];
TextLayer *text_level_ally_layer;	
TextLayer *text_level_enemy_layer;

static bool flick_animate = false;
static bool focus_animate = true;

#define ENEMY_NAME_PKEY 1
#define ALLY_NAME_PKEY 2
#define FLICK_ANIMATE_PKEY 3
#define FOCUS_ANIMATE_PKEY 4

static bool health_stats_set = false;

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

    // Timer for that delay
    if(animate)
    {
      app_timer_register(next_delay, timer_handler, NULL);
    }
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
    if(animate)
    {
      app_timer_register(next_delay, e_timer_handler, NULL);
    }
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
  if(shinyAlly)
  {
    s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ALLY_POKEMON_SHINY);
  }
  else
  {
    s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ALLY_POKEMON);
  }
  
  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);
  
  timer_handler(NULL);
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
  e_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ENEMY_POKEMON);

  // Create GBitmap
  e_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(e_sequence), GBitmapFormat8Bit);
  
  e_timer_handler(NULL);
}

void update_level_text()
{
  snprintf(level_string, sizeof(level_string), " %d", level_int);
  snprintf(level_string_2, sizeof(level_string_2), " %d", level_int_2);
	text_layer_set_text(text_level_enemy_layer, level_string_2);  
  text_layer_set_text(text_level_ally_layer, level_string);
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
  text_date_layer = text_layer_create(GRect(60, 106, 76, 10));	
	text_layer_set_text_alignment(text_date_layer, GTextAlignmentRight);
 	text_layer_set_text_color(text_date_layer, GColorBlack);
 	text_layer_set_background_color(text_date_layer, GColorClear);
 	text_layer_set_font(text_date_layer, date_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_date_layer));  
}

static void load_pokemon_name_layers(Layer *window_layer)
{  
  pokemon_name_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NAME_8));
  ally_pokemon_name_layer = text_layer_create(GRect(70,78,120,12));
  text_layer_set_text_color(ally_pokemon_name_layer, GColorBlack);
 	text_layer_set_background_color(ally_pokemon_name_layer, GColorClear);
  text_layer_set_font(ally_pokemon_name_layer, pokemon_name_font);
 	layer_add_child(window_layer, text_layer_get_layer(ally_pokemon_name_layer));
  text_layer_set_text(ally_pokemon_name_layer, ALLY_POKEMON_NAME);
  
  enemy_pokemon_name_layer = text_layer_create(GRect(5,2,120,12));
  text_layer_set_text_color(enemy_pokemon_name_layer, GColorBlack);
 	text_layer_set_background_color(enemy_pokemon_name_layer, GColorClear);
  text_layer_set_font(enemy_pokemon_name_layer, pokemon_name_font);
 	layer_add_child(window_layer, text_layer_get_layer(enemy_pokemon_name_layer));
  text_layer_set_text(enemy_pokemon_name_layer, ENEMY_POKEMON_NAME);
}

static void load_ally_pokemon_layer(Layer *window_layer)
{
  GRect bounds = layer_get_bounds(window_layer);  
  bounds.origin.x -= 50;
  bounds.origin.y += 12;

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

static void show_ally_status_sleep() 
{
  layer_set_hidden(bitmap_layer_get_layer(ally_status_sleep_layer), false);  
}

static void hide_ally_status_sleep_layer() 
{
  layer_set_hidden(bitmap_layer_get_layer(ally_status_sleep_layer), true);  
}

static void load_ally_status_sleep_layer(Layer *window_layer)
{
  ally_status_sleep_layer = bitmap_layer_create(GRect(85, 118, 50, 10));
  if(ally_status_sleep_image) {
    gbitmap_destroy(ally_status_sleep_image);
    ally_status_sleep_image = NULL;
  }
  ally_status_sleep_image = gbitmap_create_with_resource(RESOURCE_ID_STATUS_SLEEP);
  bitmap_layer_set_bitmap(ally_status_sleep_layer, ally_status_sleep_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(ally_status_sleep_layer));
  hide_ally_status_sleep_layer();
}

static void show_ally_status_par() 
{
  layer_set_hidden(bitmap_layer_get_layer(ally_status_par_layer), false);  
}

static void hide_ally_status_par_layer() 
{
  layer_set_hidden(bitmap_layer_get_layer(ally_status_par_layer), true);  
}

static void load_ally_status_par_layer(Layer *window_layer)
{
  ally_status_par_layer = bitmap_layer_create(GRect(60, 118, 50, 10));
  if(ally_status_par_image) {
    gbitmap_destroy(ally_status_par_image);
    ally_status_par_image = NULL;
  }
  ally_status_par_image = gbitmap_create_with_resource(RESOURCE_ID_STATUS_PARALYSIS);
  bitmap_layer_set_bitmap(ally_status_par_layer, ally_status_par_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(ally_status_par_layer));
  hide_ally_status_par_layer();
}

static GColor8 get_color_by_percent(uint8_t percent)
{
    GColor8 color = GColorBlack;
    if(percent > 50)
    {
       color = GColorMalachite;      
    }
    else if(percent > 25)
    {
      color = GColorChromeYellow;  
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

   	graphics_fill_rect(ctx, GRect(0, 0, (uint8_t)(battery_level)/2, 3), 0, GCornerNone);

  	if (!battery_plugged) {
      hide_ally_status_par_layer();
   	}
  	else {	
    	show_ally_status_par();
 	  }
}

static void load_battery_layer(Layer *window_layer)
{  
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

static void load_level_text_layers(Layer *window_layer)
{
  text_level_enemy_layer = text_layer_create(GRect(19, 17, 70, 12));
 	text_layer_set_text_alignment(text_level_enemy_layer, GTextAlignmentLeft);
 	text_layer_set_text_color(text_level_enemy_layer, GColorBlack);
 	text_layer_set_background_color(text_level_enemy_layer, GColorClear);
 	text_layer_set_font(text_level_enemy_layer, level_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_level_enemy_layer));

 	text_level_ally_layer = text_layer_create(GRect(85, 86, 70, 12));
 	text_layer_set_text_alignment(text_level_ally_layer, GTextAlignmentLeft);
 	text_layer_set_text_color(text_level_ally_layer, GColorBlack);
 	text_layer_set_background_color(text_level_ally_layer, GColorClear);
 	text_layer_set_font(text_level_ally_layer, level_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_level_ally_layer));
  
  update_level_text();
}

static void stop_animation()
{
  animate = false;
}

static void start_animation()
{
  animate = true;
  app_timer_register(1, timer_handler, NULL);
  app_timer_register(1, e_timer_handler, NULL);
  app_timer_register(10000, stop_animation, NULL);
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
  
  load_ally_status_sleep_layer(window_layer);
  
  load_ally_status_par_layer(window_layer);
  
  load_pokemon_name_layers(window_layer);
  
  load_level_text_layers(window_layer);
  
  if(focus_animate){
    start_animation();
  }
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
  bitmap_layer_destroy(e_bitmap_layer);
}

static void set_health_stats(){  
  HealthMetric metric = HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);
  int stepsToday = 0;
  int averageStepsByNow = 1;
  int endOfDayStepGoal = 1;
  
  // Check the metric has data available for today
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
  
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Data is available!    
    stepsToday = (int)health_service_sum_today(metric);
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps today: %d", stepsToday);
  } else {
    // No data recorded yet today
    APP_LOG(APP_LOG_LEVEL_ERROR, "Data unavailable!");
  }   
  
  // Define query parameters
  const HealthServiceTimeScope scope = HealthServiceTimeScopeWeekly;
   
  // Check that an averaged value is accessible
  mask = health_service_metric_averaged_accessible(metric, start, end, scope);
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Average is available, read it
    HealthValue average = health_service_sum_averaged(metric, start, end, scope);    
    averageStepsByNow = (int)average;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Average step count: %d steps", averageStepsByNow);
  }
  
  mask = health_service_metric_averaged_accessible(metric, start, start + 86399, scope);
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Average is available, read it
    HealthValue average = health_service_sum_averaged(metric, start, start + 86399, scope);    
    endOfDayStepGoal = (int)average;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Average step count: %d steps", averageStepsByNow);
  } else{
    endOfDayStepGoal = 10000;
  }
  
  int percentStepGoal = ((double) stepsToday / averageStepsByNow) * 100;
  int endOfDayPercentStepGoal = ((double)stepsToday / endOfDayStepGoal) * 100;
  if(endOfDayPercentStepGoal <= 0){
    endOfDayPercentStepGoal = 1;
  }
  if(percentStepGoal <= 0){
    percentStepGoal = 1;
  }
  level_int_2 = 100; //TODO change?
  if(endOfDayPercentStepGoal >= 100)
  {        
    level_int = 100;
    if(!shinyAlly){
      shinyAlly = true;
      load_sequence();
    }
  }
  else
  {
    level_int = endOfDayPercentStepGoal;
    if(shinyAlly){
      shinyAlly = false;
      load_sequence();
    }
  }
  update_level_text();
  
  hour_progression = ((1 - (double)stepsToday / averageStepsByNow)) * 100;
  layer_mark_dirty(hour_progression_layer);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{    
 	static char time_text[] = "00:00";
 	static char date_text[] = "Xxx,00.00.";  

 	char *time_format;
 	char *date_format;
  
	time_format = clock_is_24h_style()?"%T":"%I:%M";	// 0:00
	date_format = "%a,%b%e";	// Fri, Dec31	
    
  strftime(time_text, sizeof(time_text), time_format, tick_time);
 	strftime(date_text, sizeof(date_text), date_format, tick_time);  
  
  if (time_text[0] == '0') {
   		memmove(time_text, &time_text[1], sizeof(time_text) - 1);
	}  

  text_layer_set_text(text_time_layer, time_text);
 	text_layer_set_text(text_date_layer, date_text);

  if(tick_time->tm_min % 5 == 0 && !health_stats_set){
    set_health_stats();
    health_stats_set = true;
  }
  else{
    health_stats_set = false;
  }
}

void battery_state_handler(BatteryChargeState charge) {
	battery_level = charge.charge_percent;
  battery_plugged = charge.is_plugged;
  layer_mark_dirty(battery_layer);
}

static void handle_bluetooth(bool connected) {	
	if (connected) {
    hide_ally_status_sleep_layer();
		if (!initiate_watchface) {
			vibes_double_pulse();
		}
	}
	else {
		// if the watchface gets started in a disconnected state it will show the SLP-screen, but won't vibrate (that would be annoying while browsing through your watchfaces)
    show_ally_status_sleep();
		if (!initiate_watchface) {      
			vibes_enqueue_custom_pattern( (VibePattern) {
   				.durations = (uint32_t []) {100, 100, 100, 100, 100},
   				.num_segments = 5
			} );
		}	
	}
}

static void handle_focus(bool in_focus)
{
  if(focus_animate){
    start_animation();
  }
}

static void handle_tap(AccelAxisType axis, int32_t direction)
{
  if(flick_animate){
    start_animation();
  }    
}

static void save_configured_data(){
  persist_write_bool(FLICK_ANIMATE_PKEY, flick_animate);
  persist_write_bool(FOCUS_ANIMATE_PKEY, focus_animate);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "focus_animate stored as %d", focus_animate);
  persist_write_string(ENEMY_NAME_PKEY, ENEMY_POKEMON_NAME);
  persist_write_string(ALLY_NAME_PKEY, ALLY_POKEMON_NAME);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Read preferences
  Tuple *enemyName = dict_find(iter, MESSAGE_KEY_EnemyName);
  if(enemyName) {
    ENEMY_POKEMON_NAME = enemyName->value->cstring;  
    text_layer_set_text(enemy_pokemon_name_layer, ENEMY_POKEMON_NAME);
  }
  Tuple *allyName = dict_find(iter, MESSAGE_KEY_AllyName);
  if(allyName){
    ALLY_POKEMON_NAME = allyName->value->cstring;
    text_layer_set_text(ally_pokemon_name_layer, ALLY_POKEMON_NAME);
  }
  Tuple *focusAnimate = dict_find(iter, MESSAGE_KEY_FocusAnimate);
  if(focusAnimate){
    focus_animate = focusAnimate->value->int8 == 1;
    //APP_LOG(APP_LOG_LEVEL_WARNING, "focusAnimate tuple value: %zu", focusAnimate->value->int32);
    //APP_LOG(APP_LOG_LEVEL_WARNING, "focusAnimate tuple value with int8: %d", focusAnimate->value->int8);
    //APP_LOG(APP_LOG_LEVEL_WARNING, "focus_animate set %d", focus_animate);
  }
  Tuple *flickAnimate = dict_find(iter, MESSAGE_KEY_FlickAnimate);
  if(flickAnimate){
    flick_animate = flickAnimate->value->int8 == 1;
    //APP_LOG(APP_LOG_LEVEL_WARNING, "flick_animate set %d", flick_animate);
  }  
}

static void retrieve_configured_data(){
  flick_animate = persist_exists(FLICK_ANIMATE_PKEY) ? persist_read_bool(FLICK_ANIMATE_PKEY) : false;
  focus_animate = persist_exists(FOCUS_ANIMATE_PKEY) ? persist_read_bool(FOCUS_ANIMATE_PKEY) : true;
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "focus_animate read as  %d", focus_animate);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "flick_animate read as  %d", flick_animate);
  if(persist_exists(ENEMY_NAME_PKEY)){
    persist_read_string(ENEMY_NAME_PKEY, ENEMY_POKEMON_NAME, strlen(ENEMY_POKEMON_NAME) + 1);
  }
  if(persist_exists(ALLY_NAME_PKEY)){
    persist_read_string(ALLY_NAME_PKEY, ALLY_POKEMON_NAME, strlen(ALLY_POKEMON_NAME) + 1);
  }
}

void handle_init(void) {
  time_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_24));
  date_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_7));
  level_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LEVEL_10));
  
  time_t now = time(NULL);
 	struct tm *tick_time = localtime(&now);
  
  retrieve_configured_data();
  
	// Create a window 
  window = window_create();
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });   

	// Push the window
	window_stack_push(window, true); 

	handle_minute_tick(tick_time, MINUTE_UNIT);
 	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);  
  
  set_health_stats();
  
  battery_state_service_subscribe (&battery_state_handler);
  
  handle_bluetooth(bluetooth_connection_service_peek());
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  
  app_focus_service_subscribe(handle_focus);
  accel_tap_service_subscribe(&handle_tap);
  app_timer_register(10000, stop_animation, NULL);
  
  app_message_open(256, 256);
  app_message_register_inbox_received(inbox_received_handler);
  
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
  
  initiate_watchface = false;
}

void handle_deinit(void) {
  save_configured_data();
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
