#include "leds.h"
#include "config.h"

#include "driver/gpio.h"

static void configure_output_pin(int pin) {
  if (pin < 0) {
    return;
  }

  gpio_config_t config = {
      .pin_bit_mask = 1ULL << pin,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  gpio_config(&config);
  gpio_set_level(pin, 0);
}

static void set_pin(int pin, bool is_on) {
  if (pin < 0) {
    return;
  }

  gpio_set_level(pin, is_on ? 1 : 0);
}

static void set_status_led(int red_pin, int yellow_pin, int green_pin,
                           LedColor color) {
  set_pin(red_pin, color == LedColor_Red);
  set_pin(yellow_pin, color == LedColor_Yellow);
  set_pin(green_pin, color == LedColor_Green);
}

void leds_init(void) {
  configure_output_pin(ButtonLedGpio);

  configure_output_pin(UltrasonicRedLedGpio);
  configure_output_pin(UltrasonicYellowLedGpio);
  configure_output_pin(UltrasonicGreenLedGpio);

  configure_output_pin(RfidRedLedGpio);
  configure_output_pin(RfidYellowLedGpio);
  configure_output_pin(RfidGreenLedGpio);

  leds_show_idle();
}

void leds_set_button(bool is_on) { set_pin(ButtonLedGpio, is_on); }

void leds_set_ultrasonic(LedColor color) {
  set_status_led(UltrasonicRedLedGpio, UltrasonicYellowLedGpio,
                 UltrasonicGreenLedGpio, color);
}

void leds_set_rfid(LedColor color) {
  set_status_led(RfidRedLedGpio, RfidYellowLedGpio, RfidGreenLedGpio, color);
}

void leds_show_idle(void) {
  leds_set_button(false);
  leds_set_ultrasonic(LedColor_Red);
  leds_set_rfid(LedColor_Red);
}

void leds_show_waiting_for_ultrasonic(void) {
  leds_set_button(true);
  leds_set_ultrasonic(LedColor_Yellow);
  leds_set_rfid(LedColor_Red);
}

void leds_show_waiting_for_rfid(void) {
  leds_set_button(true);
  leds_set_ultrasonic(LedColor_Green);
  leds_set_rfid(LedColor_Yellow);
}

void leds_show_success(void) {
  leds_set_button(true);
  leds_set_ultrasonic(LedColor_Green);
  leds_set_rfid(LedColor_Green);
}

void leds_show_fail(void) {
  leds_set_button(true);
  leds_set_ultrasonic(LedColor_Red);
  leds_set_rfid(LedColor_Red);
}
