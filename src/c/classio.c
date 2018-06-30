#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_steps_layer;
static TextLayer *s_connection_layer;

// Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
static uint32_t const segments[] = { 200, 100, 200, 100, 200, 400, 50, 100, 50, 100, 50 };
VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00:00";

  if (tick_time->tm_min % 2){
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  }else{
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));    
  } 
  strftime(s_time_text, sizeof(s_time_text), "%H:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
  // Show the date
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
  text_layer_set_text(s_connection_layer, date_buffer);

}

static void handle_bluetooth(bool connected) {
  // text_layer_set_text(s_connection_layer, connected ? "connected" : "disconnected");
  if( connected ){
    window_set_background_color(s_main_window, GColorWhite);
    vibes_double_pulse();
  }else{ 
    window_set_background_color(s_main_window, GColorRed);
    vibes_enqueue_custom_pattern(pat);
  }
}
/*************************** Health System ************************************/
void updateSteps(){
  HealthMetric metric = HealthMetricStepCount;

  time_t end = time(NULL);
//  time_t start = time_start_of_today();
  time_t oneHour = end - SECONDS_PER_HOUR;

  static char date_buffer[26];

  // Check data is available
  HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, oneHour, end);
  if(result & HealthServiceAccessibilityMaskAvailable) {
    // Data is available! Read it
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps in the last hour: %d", (int)health_service_sum(metric, oneHour, end));
    // if the # of steps in the last hour is less then 500
    // change the color of the layer

//    text_layer_set_background_color(s_steps_layer, GColorClear);      
//    text_layer_set_text_color(s_steps_layer, settings.textColor);

    snprintf( date_buffer, sizeof(date_buffer), "%d:%d", (int)health_service_sum_today(metric), 
                                                             (int)health_service_sum(metric, oneHour, end) );
    text_layer_set_text(s_steps_layer, date_buffer);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No data available!");
  }
  
}
/*
  health system 20180201
  */
static void health_handler(HealthEventType event, void *context) {
  
  // Which type of event occurred?
  switch(event) {
    case HealthEventMetricAlert:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventMetricAlert event");
      break;
    case HealthEventSignificantUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSignificantUpdate event");
      updateSteps();
    break;
    case HealthEventMovementUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventMovementUpdate event");
      updateSteps();
      break;
    case HealthEventSleepUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSleepUpdate event");
      break;
    case HealthEventHeartRateUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO,
              "New HealthService HealthEventHeartRateUpdate event");
      break;
  }
}
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_time_layer = text_layer_create(GRect(0, 20, bounds.size.w, 54));
  text_layer_set_text_color(s_time_layer, GColorBlue);
  text_layer_set_background_color(s_time_layer, GColorClear);
//  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  s_connection_layer = text_layer_create(GRect(0, 90, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorBlack);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentCenter);
  handle_bluetooth(connection_service_peek_pebble_app_connection());

  s_battery_layer = text_layer_create(GRect(0, 120, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "100% charged");

  s_steps_layer = text_layer_create(GRect(0, -8, bounds.size.w -5, 25));
  text_layer_set_text_color(s_steps_layer, GColorBlack);
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_font(s_steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentRight);

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  battery_state_service_subscribe(handle_battery);

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_bluetooth
  });

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));

  handle_battery(battery_state_service_peek());
  
    // Subscrive the health service
  #if defined(PBL_HEALTH)
    // Attempt to subscribe 
    if(!health_service_events_subscribe(health_handler, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health NOT available!");
    }
    #else
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health available!");
  #endif
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_steps_layer);
}

static void init() {
  s_main_window = window_create();
//  window_set_background_color(s_main_window, GColorBlack);
  window_set_background_color(s_main_window, GColorWhite);
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
