#include "ultrasonic.h"
#include "config.h"

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

static const int UltrasonicTimeoutUs = 30000;

void ultrasonic_init(void) {
  gpio_config_t trig_config = {
      .pin_bit_mask = 1ULL << UltrasonicTrigGpio,
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  gpio_config_t echo_config = {
      .pin_bit_mask = 1ULL << UltrasonicEchoGpio,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  gpio_config(&trig_config);
  gpio_config(&echo_config);

  gpio_set_level(UltrasonicTrigGpio, 0);
}

int ultrasonic_get_distance_cm(void) {
  gpio_set_level(UltrasonicTrigGpio, 0);
  esp_rom_delay_us(2);

  gpio_set_level(UltrasonicTrigGpio, 1);
  esp_rom_delay_us(10);

  gpio_set_level(UltrasonicTrigGpio, 0);

  int64_t wait_start_us = esp_timer_get_time();

  while (gpio_get_level(UltrasonicEchoGpio) == 0) {
    if (esp_timer_get_time() - wait_start_us > UltrasonicTimeoutUs) {
      return -1;
    }
  }

  int64_t pulse_start_us = esp_timer_get_time();

  while (gpio_get_level(UltrasonicEchoGpio) == 1) {
    if (esp_timer_get_time() - pulse_start_us > UltrasonicTimeoutUs) {
      return -1;
    }
  }

  int64_t pulse_end_us = esp_timer_get_time();
  int64_t duration_us = pulse_end_us - pulse_start_us;

  return (int)(duration_us / 58);
}

bool ultrasonic_detected_object(int max_distance_cm) {
  int distance_cm = ultrasonic_get_distance_cm();

  if (distance_cm < DetectionMinDistanceCm) {
    return false;
  }

  return distance_cm <= max_distance_cm;
}
