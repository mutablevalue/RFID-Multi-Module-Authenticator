#include "button.h"
#include "config.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static volatile int button_press_events = 0;
static volatile TickType_t last_press_tick = 0;

static portMUX_TYPE button_mux = portMUX_INITIALIZER_UNLOCKED;

static const TickType_t ButtonDebounceTicks = pdMS_TO_TICKS(50);

static void button_isr_handler(void *arg) {
  (void)arg;

  TickType_t now_tick = xTaskGetTickCountFromISR();

  if (last_press_tick != 0 &&
      now_tick - last_press_tick < ButtonDebounceTicks) {
    return;
  }

  last_press_tick = now_tick;

  portENTER_CRITICAL_ISR(&button_mux);
  button_press_events++;
  portEXIT_CRITICAL_ISR(&button_mux);
}

void button_init(void) {
  if (ButtonInputGpio < 0) {
    return;
  }

  gpio_config_t config = {
      .pin_bit_mask = 1ULL << ButtonInputGpio,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_NEGEDGE,
  };

  gpio_config(&config);

  esp_err_t err = gpio_install_isr_service(0);

  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    return;
  }

  gpio_isr_handler_remove(ButtonInputGpio);
  gpio_isr_handler_add(ButtonInputGpio, button_isr_handler, NULL);
}

bool button_was_pressed(void) {
  bool pressed = false;

  portENTER_CRITICAL(&button_mux);

  if (button_press_events > 0) {
    button_press_events--;
    pressed = true;
  }

  portEXIT_CRITICAL(&button_mux);

  return pressed;
}
