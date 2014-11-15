#include <pebble.h>
#include <stdlib.h>
  
static Window *s_main_window;

static AppTimer* timer; // Timer
static TextLayer *s_output_layer; // TextLayer
static int buffer_size = 50;
static double s_buffer_x[50];
static double s_buffer_y[50];
static double s_buffer_z[50];
static int buffer_index = 0;

/*
static void timer_callback(void *data) {
  // Vibrates two times
  vibes_double_pulse();
  // Sets a new timer
  timer = app_timer_register(5000, timer_callback, NULL);
}*/

static void data_handler(AccelData *data, uint32_t num_samples) {
  // Store the accelerometer data
  s_buffer_x[buffer_index] = (double)data[0].x;
  s_buffer_y[buffer_index] = (double)data[0].y;
  s_buffer_z[buffer_index] = (double)data[0].z;
  
  // Increment buffer index
  buffer_index++;
  
  if (buffer_index == buffer_size) {
    double xAccel = 0;
    for (int i = 0; i < buffer_size; i++) {
      xAccel += abs(s_buffer_x[i]);
    }
    xAccel /= buffer_size;
    double yAccel = 0;
    for (int i = 0; i < buffer_size; i++) {
      yAccel += abs(s_buffer_y[i]);
    }
    yAccel /= buffer_size;
    double zAccel = 0;
    for (int i = 0; i < buffer_size; i++) {
      zAccel += abs(s_buffer_z[i]);
    }
    zAccel /= buffer_size;
    
    if (xAccel <= 75 && yAccel <= 75 && zAccel <= 75) {
      // Vibrate
      vibes_double_pulse();
    }
    
    // Resets the buffer index
    buffer_index = 0;
  }
  /*
  snprintf(s_buffer, sizeof(s_buffer), 
    "N X,Y,Z\n0 %d,%d,%d\n1 %d,%d,%d\n2 %d,%d,%d", 
    data[0].x, data[0].y, data[0].z, 
    data[1].x, data[1].y, data[1].z, 
    data[2].x, data[2].y, data[2].z
  );

  //Show the data
  text_layer_set_text(s_output_layer, s_buffer);*/
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No data yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
  
  // Creates a new timer
  //timer = app_timer_register(5000, timer_callback, NULL);
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Subscribe to the accelerometer data service
  int num_samples = 1;
  accel_data_service_subscribe(num_samples, data_handler);
  
  // Set the sampling rate for the accelerometer
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  
  // Unsubscribe from the accelerometer data service
  accel_data_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}