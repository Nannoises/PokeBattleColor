#include <pebble.h>

Window *window;
static GBitmap *s_bitmap = NULL;
static GBitmap *e_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static BitmapLayer *e_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;
static GBitmapSequence *e_sequence = NULL;

static bool animate = true;
static bool shinyAlly = false;

static GFont *time_font;
static GFont *date_font;
TextLayer *text_time_layer;
TextLayer *text_date_layer;

static GFont *pokemon_name_font;
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

#define NUM_LEVEL_PKEY  0
#define NUM_LEVEL_FRESH 5

int	X_DELTA = 35;
int	Y_DELTA = 185;
int	Z_DELTA = 185;
int YZ_DELTA_MIN = 175;
int	YZ_DELTA_MAX = 195; 

// Timer used to determine next step check
static AppTimer *timer;

// interval to check for next step (in ms)
const int ACCEL_STEP_MS = 500;
// value to auto adjust step acceptance 
const int PED_ADJUST = 2;

int X_DELTA_TEMP, Y_DELTA_TEMP, Z_DELTA_TEMP = 0;
int lastX, lastY, lastZ, currX, currY, currZ = 0;
bool validX, validY, validZ = false;
#define STEP_COUNT_PKEY 1

int step_count = 0;
bool startedSession = false;
bool did_pebble_vibrate = false;

static int step_goal = 8000;
  
static GFont *level_font;
int level_int = 1;					

static char level_string[3];
TextLayer *text_level_ally_layer;	
TextLayer *text_level_enemy_layer;	

#define LAST_DAY_SEEN_PKEY 2

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

void autoCorrectZ(){
	if (Z_DELTA > YZ_DELTA_MAX){
		Z_DELTA = YZ_DELTA_MAX; 
	} else if (Z_DELTA < YZ_DELTA_MIN){
		Z_DELTA = YZ_DELTA_MIN;
	}
}

void autoCorrectY(){
	if (Y_DELTA > YZ_DELTA_MAX){
		Y_DELTA = YZ_DELTA_MAX; 
	} else if (Y_DELTA < YZ_DELTA_MIN){
		Y_DELTA = YZ_DELTA_MIN;
	}
}

void resetUpdate() {
	lastX = currX;
	lastY = currY;
	lastZ = currZ;
	validX = false;
	validY = false;
	validZ = false;
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
    s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_CHARIZARD_BACK);
  }
  else
  {
    s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_CHARIZARD_NORM);
  }
  
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

void pedometer_update() {
	if (startedSession) {
		X_DELTA_TEMP = abs(abs(currX) - abs(lastX));
		if (X_DELTA_TEMP >= X_DELTA) {
			validX = true;
		}
		Y_DELTA_TEMP = abs(abs(currY) - abs(lastY));
		if (Y_DELTA_TEMP >= Y_DELTA) {
			validY = true;
			if (Y_DELTA_TEMP - Y_DELTA > 200){
				autoCorrectY();
				Y_DELTA = (Y_DELTA < YZ_DELTA_MAX) ? Y_DELTA + PED_ADJUST : Y_DELTA;
			} else if (Y_DELTA - Y_DELTA_TEMP > 175){
				autoCorrectY();
				Y_DELTA = (Y_DELTA > YZ_DELTA_MIN) ? Y_DELTA - PED_ADJUST : Y_DELTA;
			}
		}
		Z_DELTA_TEMP = abs(abs(currZ) - abs(lastZ));
		if (abs(abs(currZ) - abs(lastZ)) >= Z_DELTA) {
			validZ = true;
			if (Z_DELTA_TEMP - Z_DELTA > 200){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			} else if (Z_DELTA - Z_DELTA_TEMP > 175){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			}
		}
	} else {
		startedSession = true;
	}
  
  //Update UI
  if ((validX && validY && !did_pebble_vibrate) || (validX && validZ && !did_pebble_vibrate)) {
		step_count++;
    
    //update level
    int percentStepGoal = ((double)step_count / step_goal) * 100;
    if(percentStepGoal > 0 && percentStepGoal != level_int && level_int < 100)
    {      
      level_int = percentStepGoal;      
      snprintf(level_string, sizeof(level_string), " %d", level_int);
	    text_layer_set_text(text_level_enemy_layer, level_string);  
	    text_layer_set_text(text_level_ally_layer, level_string);
      if(level_int == 100)
      {
        shinyAlly = true;
        load_sequence();
      }
    }
  }  
  
  resetUpdate();
}

static void step_timer_callback(void *data) {
	AccelData accel = (AccelData ) { .x = 0, .y = 0, .z = 0 };
	accel_service_peek(&accel);

	if (!startedSession) {
		lastX = accel.x;
		lastY = accel.y;
		lastZ = accel.z;
	} else {
		currX = accel.x;
		currY = accel.y;
		currZ = accel.z;
	}
	
	did_pebble_vibrate = accel.did_vibrate;

	pedometer_update();	
	
	timer = app_timer_register(ACCEL_STEP_MS, step_timer_callback, NULL);
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

static void load_pokemon_name_layers(Layer *window_layer)
{  
  pokemon_name_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NAME_8));
  ally_pokemon_name_layer = text_layer_create(GRect(70,78,120,12));
  text_layer_set_text_color(ally_pokemon_name_layer, GColorBlack);
 	text_layer_set_background_color(ally_pokemon_name_layer, GColorClear);
  text_layer_set_font(ally_pokemon_name_layer, pokemon_name_font);
 	layer_add_child(window_layer, text_layer_get_layer(ally_pokemon_name_layer));
  text_layer_set_text(ally_pokemon_name_layer, "CHARIZARD");
  
  enemy_pokemon_name_layer = text_layer_create(GRect(5,2,120,12));
  text_layer_set_text_color(enemy_pokemon_name_layer, GColorBlack);
 	text_layer_set_background_color(enemy_pokemon_name_layer, GColorClear);
  text_layer_set_font(enemy_pokemon_name_layer, pokemon_name_font);
 	layer_add_child(window_layer, text_layer_get_layer(enemy_pokemon_name_layer));
  text_layer_set_text(enemy_pokemon_name_layer, "BLASTOISE");
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
  
  snprintf(level_string, sizeof(level_string), " %d", level_int);
  text_layer_set_text(text_level_enemy_layer, level_string);  
  text_layer_set_text(text_level_ally_layer, level_string);
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
  
  if(!initiate_watchface && tick_time->tm_min == 0 && tick_time->tm_hour == 0)
  {
      step_count = 0;
      level_int = 1;
      shinyAlly = false;
      load_sequence();
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

static void stop_animation()
{
  animate = false;
}

static void handle_tap(AccelAxisType axis, int32_t direction)
{
  animate = true;
  app_timer_register(1, timer_handler, NULL);
  app_timer_register(1, e_timer_handler, NULL);
  app_timer_register(10000, stop_animation, NULL);
}

static void handle_focus(bool in_focus)
{
  animate = true;
  app_timer_register(1, timer_handler, NULL);
  app_timer_register(1, e_timer_handler, NULL);
  app_timer_register(10000, stop_animation, NULL);
}

void handle_init(void) {
  time_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_24));
  date_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_7));
  level_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LEVEL_10));
  
  time_t now = time(NULL);
 	struct tm *tick_time = localtime(&now);
    
  int last_day_seen = persist_exists(LAST_DAY_SEEN_PKEY) ? persist_read_int(LAST_DAY_SEEN_PKEY) : 0;  
  
  //Different day since last boot
  if(last_day_seen != tick_time->tm_yday)
  {
    step_count = 0;
    persist_write_int(LAST_DAY_SEEN_PKEY, tick_time->tm_yday);
  }
  else
  {
    step_count = persist_exists(STEP_COUNT_PKEY) ? persist_read_int(STEP_COUNT_PKEY) : 0;
  }
  
  int percentStepGoal = ((double)step_count / step_goal) * 100;
  if(percentStepGoal >= 100)
  {    
    shinyAlly = true;
    level_int = 100;
  }
  else if(percentStepGoal > 0)
  {
    level_int = percentStepGoal;
  }
  
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
  
  battery_state_service_subscribe (&battery_state_handler);
  
  handle_bluetooth(bluetooth_connection_service_peek());
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  
  accel_tap_service_subscribe(&handle_tap);
  app_focus_service_subscribe(handle_focus);
  app_timer_register(10000, stop_animation, NULL);
  
  //Start pedometer timer
  timer = app_timer_register(ACCEL_STEP_MS, step_timer_callback, NULL);
  
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
  
  initiate_watchface = false;
}

void handle_deinit(void) {
	// TODO destroy stuff
  app_timer_cancel(timer);  
  
	// Destroy the window
	window_destroy(window);
  
  time_t now = time(NULL);
 	struct tm *tick_time = localtime(&now);
   
  persist_write_int(LAST_DAY_SEEN_PKEY, tick_time->tm_yday);
  persist_write_int(STEP_COUNT_PKEY, step_count);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
