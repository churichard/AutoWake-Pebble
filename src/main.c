#include <pebble.h>
#include <stdlib.h>
#include "const.h"

//extent of array used. Must be less than MAX_BUFFER_SIZE.
int userBufferSize;

//settings maintained by const.h
extern int bVibration;    // boolean on/off
extern int bSound;        // boolean on/off
extern int bSwitch;       // boolean on/off 
extern int iSensitivity;  // integer values 0 to 400
extern int iDelay;        // seconds

//Vibration patterns
enum {NUM_VIBES= 12};
uint32_t segments[NUM_VIBES];

static Window *s_main_window;

// static AppTimer* timer; // Timer
static TextLayer *s_output_layer; // TextLayer
static int a_buffer_x[MAX_BUFFER_SIZE];
static int a_buffer_y[MAX_BUFFER_SIZE];
static int a_buffer_z[MAX_BUFFER_SIZE];

static int j_buffer_x[MAX_BUFFER_SIZE];
static int j_buffer_y[MAX_BUFFER_SIZE];
static int j_buffer_z[MAX_BUFFER_SIZE];

/*
Test Code that gives acceleration readings
*/
int main2(void);  //TODO: Remove. Temporary tests

//External Functions
void inbox_received_callback(DictionaryIterator *iterator, void *context);
void inbox_dropped_callback(AppMessageResult reason, void *context);
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void outbox_sent_callback(DictionaryIterator *iterator, void *context);

/* 
 * Vibrate with a random vibration scheme
 */
static void vibrate() {
  for(int i = 0; i < NUM_VIBES; i += 2) {
    segments[i] = rand() % 300 + 100;
    segments[i+1] = rand() % segments[i] + 100;
  }
  VibePattern pat = { 
    .durations = segments,
    .num_segments = NUM_VIBES,
  };
  vibes_enqueue_custom_pattern(pat);
}

/* 
 * Call on the phone to ring
 */
static void ring() {
  //Ring the phone
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet ringTuplet = TupletInteger(RING, 2550);
  dict_write_tuplet(iter, &ringTuplet);
  app_message_outbox_send();
}

// static void timer_callback(void *data) {
//   // Vibrates two times
//   vibes_double_pulse();
//   // Sets a new timer
//   timer = app_timer_register(5000, timer_callback, NULL);
// }
static int buffer_index = 0;
static int firstPass = 1; //true for first pass through array
static int firstA = 1;    //true for first acceleration reading
static int handlerKey = 0;
static void data_handler(AccelData *data, uint32_t num_samples) {
  handlerKey = 1;
  // Store the accelerometer data
  a_buffer_x[buffer_index] = data[0].x;
  a_buffer_y[buffer_index] = data[0].y;
  a_buffer_z[buffer_index] = data[0].z;
  
  if (buffer_index == 0 && !firstA) {
    j_buffer_x[buffer_index] = (a_buffer_x[buffer_index] - a_buffer_x[userBufferSize-1]) * SAMPLING_RATE / FACTOR;
    j_buffer_y[buffer_index] = (a_buffer_y[buffer_index] - a_buffer_y[userBufferSize-1]) * SAMPLING_RATE / FACTOR;
    j_buffer_z[buffer_index] = (a_buffer_z[buffer_index] - a_buffer_z[userBufferSize-1]) * SAMPLING_RATE / FACTOR;
  }
  else if(!firstA) {
    j_buffer_x[buffer_index] = (a_buffer_x[buffer_index] - a_buffer_x[buffer_index-1]) * SAMPLING_RATE / FACTOR;
    j_buffer_y[buffer_index] = (a_buffer_y[buffer_index] - a_buffer_y[buffer_index-1]) * SAMPLING_RATE / FACTOR;
    j_buffer_z[buffer_index] = (a_buffer_z[buffer_index] - a_buffer_z[buffer_index-1]) * SAMPLING_RATE / FACTOR;
  }
  else {
    firstA = 0;
  }
  
  /*
  * Jolt values are taken to be the squares of the 
  * actual jolts
  */
  int xJolt = 0;
  int yJolt = 0;
  int zJolt = 0;
  for (int i = 0; i < userBufferSize; i++) {
    xJolt += j_buffer_x[i]*j_buffer_x[i];
    yJolt += j_buffer_y[i]*j_buffer_y[i];
    zJolt += j_buffer_z[i]*j_buffer_z[i];
  }
  //square of the average magnitude of acceleration
  int jolt = (xJolt + yJolt + zJolt) / userBufferSize;
  
  if ((abs(jolt - STILL) < iSensitivity) && !firstPass) {
    if (bVibration) {
      vibrate();
    }
    if (bSound) {
      ring();
    }
  }
  
  // Increment buffer index
  buffer_index++;
  if (buffer_index >= userBufferSize) { 
    // Resets the buffer index
    buffer_index = 0;
    // Not first
    firstPass = 0;
  }
  
  
  // Long lived buffer
  static char message[128];
  // Compose string of all data
  snprintf(message, sizeof(message), 
    "X: %d  \nY: %d  \nZ: %d\nAverage Jolt: \n%d\nFirst: %s", 
    xJolt/userBufferSize, yJolt/userBufferSize, zJolt/userBufferSize,
    jolt - STILL,
    firstPass ? "True" : "False"
  );

  //Show the data
  text_layer_set_text(s_output_layer, message);
  handlerKey = 0;
}
void reset_data_handler() {
  while(handlerKey);
  userBufferSize = SAMPLING_RATE * iDelay;
  firstPass = 1; //since the values in the array are not full yet
//   buffer_index = 0;
//   firstA = 1;    //true for first acceleration reading
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
  
  //Set the default sensitivity
  iSensitivity = (400) * SAMPLING_RATE * SAMPLING_RATE / FACTOR / FACTOR;
  
  //Change the accelerometer sampling rate. Not Needed.
  //accel_service_set_samples_per_update(num_samples);
  
  int userDelay = 5;
  
  // Set the sampling rate for the accelerometer
  accel_service_set_sampling_rate(SAMPLING_RATE);
  
  userBufferSize = SAMPLING_RATE * userDelay;
  
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  
  // Unsubscribe from the accelerometer data service
  accel_data_service_unsubscribe();
}

int main(void) {
//   main2();
//   return 0;
  init();
  app_event_loop();
  deinit();
}