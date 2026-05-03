#pragma once

#include <stdbool.h>

typedef enum {
  LedColor_Off,
  LedColor_Red,
  LedColor_Yellow,
  LedColor_Green,
} LedColor;

void leds_init(void);

void leds_set_button(bool is_on);
void leds_set_ultrasonic(LedColor color);
void leds_set_rfid(LedColor color);

void leds_show_idle(void);
void leds_show_waiting_for_ultrasonic(void);
void leds_show_waiting_for_rfid(void);
void leds_show_success(void);
void leds_show_fail(void);
