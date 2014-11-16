#include <pebble.h>
#include "const.h"
//enum {VIBRATION = 'v', SOUND = 's', SWITCH = 'w', ACC = 0, TIM = 1, SENSITIVITY = 'e', DELAY = 'd'};

int bVibration = 1;    // boolean on/off
int bSound = 0;        // boolean on/off
int bSwitch = ACC;       // boolean on/off 
int iSensitivity;  // integer values 0 to 400. Initialized by init() in main.c
int iDelay = 60;        // seconds

//function from main.c
int reset_data_handler();

void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while(t != NULL) {
    // Process this pair's key
    switch (t->key) {
      case VIBRATION:
        bVibration = (int)t->value->int32;
        break;
      case SOUND:
        bSound = (int)t->value->int32;
        break;
      case SWITCH:
        bSwitch = (int)t->value->int32; //TODO
        break;
      case SENSITIVITY:
        iSensitivity = (200 + (int)t->value->int32) * SAMPLING_RATE * SAMPLING_RATE / FACTOR / FACTOR;
        break;
      case DELAY:
        iDelay = (int)t->value->int32;
        reset_data_handler();
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_DATA received with value %d", (int)t->value->int32);
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}