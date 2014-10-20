#include <pebble.h>

static Window *window;

static TextLayer *time_layer;
static TextLayer *countdown_layer;

static char time_text[] = "00:00";
static char countdown_text[] = "remaining 00 min";

static bool is_started = false;
static uint16_t duration_minutes = 26;
static uint16_t countdown_minutes;
static AppTimer *timer;

static void timer_callback(void *data) {
  vibes_double_pulse();
	is_started = false;
	text_layer_set_text(countdown_layer, "press select");
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
	if(clock_is_24h_style()) {
		strftime(time_text, sizeof(time_text), "%H:%M", tick_time);
	}
	else {
		strftime(time_text, sizeof(time_text), "%I:%M", tick_time);	
		if (time_text[0] == '0') {
			memmove(&time_text[0], &time_text[1], sizeof(time_text) - 1); //remove leading zero
		}
	}    
	text_layer_set_text(time_layer, time_text);
	
	if(is_started) {
		snprintf(countdown_text, sizeof(countdown_text), "remaining %d min", countdown_minutes);
		countdown_minutes--;
	}
	
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(is_started){
		//stop it
		text_layer_set_text(countdown_layer, "press select");
		if(timer) {
			app_timer_cancel(timer);
		}
	}
	else {
		//start it		
		countdown_minutes = duration_minutes;
		snprintf(countdown_text, sizeof(countdown_text), "remaining %d min", countdown_minutes);
		text_layer_set_text(countdown_layer, countdown_text);
		timer = app_timer_register((duration_minutes * 60 * 1000) /* milliseconds */, timer_callback, NULL);
	}
	is_started = !is_started;
}

static void locked_click_handler(ClickRecognizerRef recognizer, void *context) {}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, locked_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, locked_click_handler);
	window_single_click_subscribe(BUTTON_ID_BACK, locked_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
	window_set_background_color(window, GColorBlack);

  //time_layer
  time_layer = text_layer_create(GRect(0, 55, 144, 45));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_text(time_layer, "00:00");
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, (Layer *)time_layer);	
	
  //countdown_layer
  countdown_layer = text_layer_create(GRect(0, 130, 144, 40));
  text_layer_set_background_color(countdown_layer, GColorClear);
  text_layer_set_text_color(countdown_layer, GColorWhite);
  text_layer_set_text(countdown_layer, "press select");
  text_layer_set_text_alignment(countdown_layer, GTextAlignmentCenter);
  text_layer_set_font(countdown_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, (Layer *)countdown_layer);	
	
	//Somebody set us up the CLOCK
	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);  
	
	if(tick_time->tm_sec>=30) {
		duration_minutes++; 
	}

	handle_tick(tick_time, MINUTE_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);		
}

static void window_unload(Window *window) {
	text_layer_destroy(time_layer);
	text_layer_destroy(countdown_layer);
}

static void init(void) {
  window = window_create();
	window_set_fullscreen(window, true);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	  .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}