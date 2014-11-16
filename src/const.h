#pragma once
#include <pebble.h>

//-------Constants from main.c--------//
enum {MAX_BUFFER_SIZE = 600, SAMPLING_RATE = ACCEL_SAMPLING_10HZ, FACTOR = 10};
enum {STILL = 0, MARGIN = 400 * SAMPLING_RATE * SAMPLING_RATE / FACTOR / FACTOR};
enum {RING = 'r'};


//-------Constants from comm.c--------//
enum {VIBRATION = 'v', SOUND = 's', SWITCH = 'w', ACC = 0, TIM = 1, SENSITIVITY = 'e', DELAY = 'd'};