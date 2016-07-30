#include <pebble.h>

Window *window;
static GBitmap *s_bitmap = NULL;
static GBitmap *e_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static BitmapLayer *e_bitmap_layer;

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

static GBitmap *terrain_image;
static BitmapLayer *terrain_layer;

static uint8_t hour_progression;
static Layer *hour_progression_layer;

static GBitmap *ally_status_sleep_image;
static BitmapLayer *ally_status_sleep_layer;

static GBitmap *ally_status_par_image;
static BitmapLayer *ally_status_par_layer;

static bool initiate_watchface = true;

char *ALLY_POKEMON_NAME; 
char *ENEMY_POKEMON_NAME;
int pokemon_name_max_length = 10;
  
static GFont level_font;
int level_int = 1;
int level_int_2 = 1;

static char level_string[10];
static char level_string_2[10];
TextLayer *text_level_ally_layer;	
TextLayer *text_level_enemy_layer;

#define ENEMY_NAME_PKEY 1
#define ALLY_NAME_PKEY 2
#define ALLY_SPRITE_PKEY 3
#define ALLY_SHINY_SPRITE_PKEY 4
#define ENEMY_SPRITE_PKEY 5
#define SPRITE_SIZES_PKEY 6

#define IMAGE_TYPE_ALLY_SPRITE 0
#define IMAGE_TYPE_ALLY_SHINY_SPRITE 1
#define IMAGE_TYPE_ENEMY_SPRITE 2

static bool health_stats_set = false;

#define NUMBER_OF_POKEMON_SPRITES 3

static uint8_t *img_data[NUMBER_OF_POKEMON_SPRITES];
static int img_size[NUMBER_OF_POKEMON_SPRITES];
static bool img_loaded[NUMBER_OF_POKEMON_SPRITES];

//static int test_size;
//static uint8_t *test_data;

void update_level_text()
{
  snprintf(level_string, sizeof(level_string), " %d", level_int);
  snprintf(level_string_2, sizeof(level_string_2), " %d", level_int_2);
	text_layer_set_text(text_level_enemy_layer, level_string_2);  
  text_layer_set_text(text_level_ally_layer, level_string);
}

static void load_time_text_layer(Layer *window_layer)
{  
  GRect window_layer_frame = layer_get_frame(window_layer);
  #if defined(PBL_RECT)
    GRect text_layer_frame = GRect(window_layer_frame.size.w / 12, (window_layer_frame.size.h * 11) / 14, (window_layer_frame.size.w * 5) / 6 , window_layer_frame.size.h / 6);
  #elif defined(PBL_ROUND)
    GRect text_layer_frame = GRect((window_layer_frame.size.w / 12) - 2, ((window_layer_frame.size.h * 3) / 4) + 2, ((window_layer_frame.size.w * 5) / 6) + 4, window_layer_frame.size.h / 4);
  #endif  
  text_time_layer = text_layer_create(text_layer_frame);
 	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
 	text_layer_set_text_color(text_time_layer, GColorBlack);
 	text_layer_set_background_color(text_time_layer, GColorFromRGB(85, 170, 170));
 	text_layer_set_font(text_time_layer, time_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));  
}

static void load_date_text_layer(Layer *window_layer)
{
  #if defined(PBL_RECT)
    text_date_layer = text_layer_create(GRect(60, 106, 76, 10));	
  #elif defined(PBL_ROUND)
    text_date_layer = text_layer_create(GRect(85, 110, 76, 10));
  #endif  
	text_layer_set_text_alignment(text_date_layer, GTextAlignmentRight);
 	text_layer_set_text_color(text_date_layer, GColorBlack);
 	text_layer_set_background_color(text_date_layer, GColorClear);
 	text_layer_set_font(text_date_layer, date_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_date_layer));  
}

static void load_pokemon_name_layers(Layer *window_layer)
{  
  pokemon_name_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NAME_8));
  #if defined(PBL_RECT)
    ally_pokemon_name_layer = text_layer_create(GRect(70,78,120,12));
  #elif defined(PBL_ROUND)
    ally_pokemon_name_layer = text_layer_create(GRect(95,84,120,12));
  #endif

  text_layer_set_text_color(ally_pokemon_name_layer, GColorBlack);
 	text_layer_set_background_color(ally_pokemon_name_layer, GColorClear);
  text_layer_set_font(ally_pokemon_name_layer, pokemon_name_font);
 	layer_add_child(window_layer, text_layer_get_layer(ally_pokemon_name_layer));
  text_layer_set_text(ally_pokemon_name_layer, ALLY_POKEMON_NAME);
  
  #if defined(PBL_RECT)
    enemy_pokemon_name_layer = text_layer_create(GRect(5,2,120,12));
  #elif defined(PBL_ROUND)
    enemy_pokemon_name_layer = text_layer_create(GRect(37,15,120,12));
  #endif  
  text_layer_set_text_color(enemy_pokemon_name_layer, GColorBlack);
 	text_layer_set_background_color(enemy_pokemon_name_layer, GColorClear);
  text_layer_set_font(enemy_pokemon_name_layer, pokemon_name_font);
 	layer_add_child(window_layer, text_layer_get_layer(enemy_pokemon_name_layer));
  text_layer_set_text(enemy_pokemon_name_layer, ENEMY_POKEMON_NAME);
}

static void load_ally_pokemon_layer(Layer *window_layer)
{
  GRect bounds = layer_get_bounds(window_layer);  
  bounds.origin.x -= 45;
  bounds.origin.y += 47;  
  bounds.size.h -= 85;

  s_bitmap_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
}

static void load_enemy_pokemon_layer(Layer *window_layer)
{
  GRect e_bounds = layer_get_bounds(window_layer);
  e_bounds.origin.x += 35;
  e_bounds.origin.y -= 50;  
  
  e_bitmap_layer = bitmap_layer_create(e_bounds);
  bitmap_layer_set_compositing_mode(e_bitmap_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(e_bitmap_layer));  
}

static void load_ally_image()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Attempting to set s_bitmap. Bytes used: %zu Bytes free: %zu", heap_bytes_used(), heap_bytes_free());
  if(s_bitmap){        
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying ally bitmap.");
    gbitmap_destroy(s_bitmap);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "s_bitmap destroyed. Bytes used: %zu Bytes free: %zu", heap_bytes_used(), heap_bytes_free());
  }
  
  // Create new GBitmap from downloaded PNG data
  if(shinyAlly){
    if(img_loaded[IMAGE_TYPE_ALLY_SHINY_SPRITE]){
      s_bitmap = gbitmap_create_from_png_data(img_data[IMAGE_TYPE_ALLY_SHINY_SPRITE], img_size[IMAGE_TYPE_ALLY_SHINY_SPRITE]);  
    } else {
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BALL);
    }        
  } else{
    if(img_loaded[IMAGE_TYPE_ALLY_SPRITE]){
      s_bitmap = gbitmap_create_from_png_data(img_data[IMAGE_TYPE_ALLY_SPRITE], img_size[IMAGE_TYPE_ALLY_SPRITE]);      
    } else {      
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BALL);
    }
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "s_bitmap set. Bytes used: %zu Bytes free: %zu", heap_bytes_used(), heap_bytes_free());

  // Show the image
  if(s_bitmap) {
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No Ally GBitmap!");
  }
}

static void load_enemy_image()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Attempting to set e_bitmap. Bytes used: %zu Bytes free: %zu", heap_bytes_used(), heap_bytes_free());
  if(e_bitmap){   
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying enemy bitmap.");
    gbitmap_destroy(e_bitmap);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "e_bitmap destroyed. Bytes used: %zu Bytes free: %zu", heap_bytes_used(), heap_bytes_free());
  }
  
  // Create new GBitmap from downloaded PNG data
  if(img_loaded[IMAGE_TYPE_ENEMY_SPRITE]){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Attempting to set e_bitmap to img of size: %d and first value of %d", img_size[IMAGE_TYPE_ENEMY_SPRITE],img_data[IMAGE_TYPE_ENEMY_SPRITE][0]);  
    e_bitmap = gbitmap_create_from_png_data(img_data[IMAGE_TYPE_ENEMY_SPRITE], img_size[IMAGE_TYPE_ENEMY_SPRITE]);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Attempting to set e_bitmap to img of size: %d and first value of %d", test_size,test_data[0]);  
    //e_bitmap = gbitmap_create_from_png_data(test_data, test_size);
  } else {
    e_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BALL);    
  }  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "e_bitmap set. Bytes used: %zu Bytes free: %zu", heap_bytes_used(), heap_bytes_free());

  // Show the image
  if(e_bitmap) {
    bitmap_layer_set_bitmap(e_bitmap_layer, e_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(e_bitmap_layer));
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No Enemy GBitmap!");
  }
}

static void load_background_layer(Layer *window_layer)
{
  background_image = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  GRect background_frame = layer_get_frame(window_layer);
  #if defined(PBL_RECT)    
  #elif defined(PBL_ROUND)
    background_frame.size.h = 137;
  #endif  
  background_layer = bitmap_layer_create(background_frame);
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
  #if defined(PBL_RECT)
    ally_status_sleep_layer = bitmap_layer_create(GRect(85, 118, 50, 10));
  #elif defined(PBL_ROUND)
    ally_status_sleep_layer = bitmap_layer_create(GRect(107, 122, 50, 10));
  #endif    
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
  #if defined(PBL_RECT)
     ally_status_par_layer = bitmap_layer_create(GRect(60, 118, 50, 10));
  #elif defined(PBL_ROUND)
     ally_status_par_layer = bitmap_layer_create(GRect(82, 122, 50, 10));
  #endif  
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
  #if defined(PBL_RECT)
     	battery_layer = layer_create(GRect(88,100,50,3));
  #elif defined(PBL_ROUND)
     	battery_layer = layer_create(GRect(115,104,50,3));
  #endif  
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
  #if defined(PBL_RECT)
    hour_progression_layer = layer_create(GRect(21, 31, 50, 3));
  #elif defined(PBL_ROUND)
    hour_progression_layer = layer_create(GRect(41, 35, 50, 3));
  #endif    
  layer_set_update_proc(hour_progression_layer, &hour_progression_layer_update_callback);
  layer_add_child(window_layer, hour_progression_layer);
}

static void load_level_text_layers(Layer *window_layer)
{
  #if defined(PBL_RECT)
    text_level_enemy_layer = text_layer_create(GRect(19, 17, 70, 12));	
  #elif defined(PBL_ROUND)
    text_level_enemy_layer = text_layer_create(GRect(35, 21, 70, 12));
  #endif    
 	text_layer_set_text_alignment(text_level_enemy_layer, GTextAlignmentLeft);
 	text_layer_set_text_color(text_level_enemy_layer, GColorBlack);
 	text_layer_set_background_color(text_level_enemy_layer, GColorClear);
 	text_layer_set_font(text_level_enemy_layer, level_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_level_enemy_layer));

  #if defined(PBL_RECT)
    text_level_ally_layer = text_layer_create(GRect(85, 86, 70, 12));
  #elif defined(PBL_ROUND)
    text_level_ally_layer = text_layer_create(GRect(110, 90, 70, 12));
  #endif   	
 	text_layer_set_text_alignment(text_level_ally_layer, GTextAlignmentLeft);
 	text_layer_set_text_color(text_level_ally_layer, GColorBlack);
 	text_layer_set_background_color(text_level_ally_layer, GColorClear);
 	text_layer_set_font(text_level_ally_layer, level_font);
 	layer_add_child(window_layer, text_layer_get_layer(text_level_ally_layer));
  
  update_level_text();
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
      load_ally_image();
    }
  }
  else
  {
    level_int = endOfDayPercentStepGoal;
    if(shinyAlly){
      shinyAlly = false;
      load_ally_image();
    }
  }
  update_level_text();
  
  hour_progression = ((1 - (double)stepsToday / averageStepsByNow)) * 100;  
  layer_mark_dirty(hour_progression_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  load_background_layer(window_layer);
  
  load_ally_pokemon_layer(window_layer);
  
  load_enemy_pokemon_layer(window_layer);
    
  load_battery_layer(window_layer);
  
  load_hour_progression_layer(window_layer);
  
  load_time_text_layer(window_layer);
  
  load_date_text_layer(window_layer);
  
  load_ally_status_sleep_layer(window_layer);
  
  load_ally_status_par_layer(window_layer);
  
  load_pokemon_name_layers(window_layer);
  
  load_level_text_layers(window_layer);
  
  set_health_stats();
  
  load_ally_image();
  
  load_enemy_image();
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
  bitmap_layer_destroy(e_bitmap_layer);
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

static void save_configured_data(){
  persist_write_string(ENEMY_NAME_PKEY, ENEMY_POKEMON_NAME);
  persist_write_string(ALLY_NAME_PKEY, ALLY_POKEMON_NAME);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Read preferences
  Tuple *enemyName = dict_find(iter, MESSAGE_KEY_EnemyName);
  if(enemyName) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found enemyName. %s", enemyName->value->cstring); 
    free(ENEMY_POKEMON_NAME);
    ENEMY_POKEMON_NAME = (char *)malloc((pokemon_name_max_length + 1) * sizeof(char));
    strncpy(ENEMY_POKEMON_NAME, enemyName->value->cstring, (pokemon_name_max_length + 1));    
    text_layer_set_text(enemy_pokemon_name_layer, ENEMY_POKEMON_NAME);
  }
  Tuple *allyName = dict_find(iter, MESSAGE_KEY_AllyName);
  if(allyName){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found allyName. %s", allyName->value->cstring);    
    free(ALLY_POKEMON_NAME);
    ALLY_POKEMON_NAME = (char *)malloc((pokemon_name_max_length + 1)* sizeof(char));    
    strncpy(ALLY_POKEMON_NAME, allyName->value->cstring, (pokemon_name_max_length + 1));    
    text_layer_set_text(ally_pokemon_name_layer, ALLY_POKEMON_NAME);
  }
  Tuple *image_type;
  Tuple *img_size_t = dict_find(iter, MESSAGE_KEY_AppKeyDataLength);
  if(img_size_t) {    
    image_type = dict_find(iter, MESSAGE_KEY_ImageType);
    img_size[image_type->value->int32] = img_size_t->value->int32;
    //test_size = img_size_t->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Image %ld size: %d", image_type->value->int32, img_size[image_type->value->int32]);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Image %ld size: %d", image_type->value->int32, test_size);
    
    // Allocate buffer for image data
    free(img_data[image_type->value->int32]);
    img_data[image_type->value->int32] = (uint8_t*)malloc(img_size[image_type->value->int32] * sizeof(uint8_t));    
    //test_data = (uint8_t*)malloc(test_size * sizeof(uint8_t));    
  }

  // An image chunk
  Tuple *chunk_t = dict_find(iter, MESSAGE_KEY_AppKeyDataChunk);
  if(chunk_t) {
    uint8_t *chunk_data = chunk_t->value->data;
    image_type = dict_find(iter, MESSAGE_KEY_ImageType);
    Tuple *chunk_size_t = dict_find(iter, MESSAGE_KEY_AppKeyChunkSize);
    int chunk_size = chunk_size_t->value->int32;

    Tuple *index_t = dict_find(iter, MESSAGE_KEY_AppKeyIndex);
    int index = index_t->value->int32;

    // Save the chunk
    memcpy(&img_data[image_type->value->int32][index], chunk_data, chunk_size);    
    //memcpy(&test_data[index], chunk_data, chunk_size);
  }

  // Complete?
  Tuple *complete_t = dict_find(iter, MESSAGE_KEY_AppKeyComplete);
  if(complete_t) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Image completely transferred. Bytes used: %zu Bytes free: %zu", heap_bytes_used(), heap_bytes_free());        

    image_type = dict_find(iter, MESSAGE_KEY_ImageType);    
    //Show the image
    img_loaded[image_type->value->int32] = true;
    if(image_type->value->int32 == IMAGE_TYPE_ALLY_SPRITE || image_type->value->int32 == IMAGE_TYPE_ALLY_SHINY_SPRITE){
      load_ally_image();
    } else if(image_type->value->int32 == IMAGE_TYPE_ENEMY_SPRITE){
      load_enemy_image();
    }  
  }
}

static void retrieve_configured_data(){
  free(ENEMY_POKEMON_NAME);
  ENEMY_POKEMON_NAME = (char *)malloc((pokemon_name_max_length + 1) * sizeof(char));
  if(persist_exists(ENEMY_NAME_PKEY)){    
    persist_read_string(ENEMY_NAME_PKEY, ENEMY_POKEMON_NAME, pokemon_name_max_length + 1);
  }
  else{
    strcpy(ENEMY_POKEMON_NAME, "BLASTOISE");
  }
  free(ALLY_POKEMON_NAME);
  ALLY_POKEMON_NAME = (char *)malloc((pokemon_name_max_length + 1)* sizeof(char));
  if(persist_exists(ALLY_NAME_PKEY)){
    persist_read_string(ALLY_NAME_PKEY, ALLY_POKEMON_NAME, pokemon_name_max_length + 1);
  }
  else{
    strcpy(ALLY_POKEMON_NAME, "CHARIZARD");
  }
  
  //Retrieve image data TODO  
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
  
  battery_state_service_subscribe (&battery_state_handler);
  
  handle_bluetooth(bluetooth_connection_service_peek());
  bluetooth_connection_service_subscribe(&handle_bluetooth);
  
  app_message_open(app_message_inbox_size_maximum(), 64);
  app_message_register_inbox_received(inbox_received_handler);
  
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
  
  initiate_watchface = false;
}

void handle_deinit(void) {
  save_configured_data();
  if(s_bitmap){    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying ally bitmap.");
    gbitmap_destroy(s_bitmap);
  }
  if(e_bitmap){    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying enemy bitmap.");
    gbitmap_destroy(e_bitmap);
  }
    
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
