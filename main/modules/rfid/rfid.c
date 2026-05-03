#include "rfid.h"
#include "config.h"

#include <inttypes.h>
#include <string.h>

#include "esp_log.h"
#include "rc522.h"

static const char *Tag = "rfid";

static rc522_handle_t scanner;

static bool card_present = false;
static uint8_t current_uid[8];
static int current_uid_len = 0;

static void store_serial_number(uint64_t serial_number) {
  current_uid_len = 8;

  for (int i = 0; i < current_uid_len; i++) {
    current_uid[i] = (serial_number >> (8 * (current_uid_len - 1 - i))) & 0xFF;
  }
}

static void on_rfid_event(void *arg, esp_event_base_t base, int32_t event_id,
                          void *event_data) {
  rc522_event_data_t *data = (rc522_event_data_t *)event_data;

  switch (event_id) {
  case RC522_EVENT_TAG_SCANNED: {
    rc522_tag_t *tag = (rc522_tag_t *)data->ptr;

    card_present = true;
    store_serial_number(tag->serial_number);

    ESP_LOGI(Tag, "Card scanned: %" PRIu64, tag->serial_number);
    break;
  }

  default:
    break;
  }
}

void rfid_init(void) {
  rc522_config_t config = {
      .spi.host = SPI2_HOST,
      .spi.miso_gpio = RfidMisoGpio,
      .spi.mosi_gpio = RfidMosiGpio,
      .spi.sck_gpio = RfidSckGpio,
      .spi.sda_gpio = RfidCsGpio,
  };

  ESP_ERROR_CHECK(rc522_create(&config, &scanner));
  ESP_ERROR_CHECK(
      rc522_register_events(scanner, RC522_EVENT_ANY, on_rfid_event, NULL));
  ESP_ERROR_CHECK(rc522_start(scanner));

  ESP_LOGI(Tag, "RFID scanner started");
}

bool rfid_card_present(void) { return card_present; }

int rfid_get_uid(uint8_t *buffer, int max_len) {
  if (!card_present || buffer == NULL || current_uid_len <= 0) {
    return 0;
  }

  int copy_len = current_uid_len;

  if (copy_len > max_len) {
    copy_len = max_len;
  }

  memcpy(buffer, current_uid, copy_len);

  return copy_len;
}

void rfid_clear_card(void) {
  card_present = false;
  current_uid_len = 0;
  memset(current_uid, 0, sizeof(current_uid));
}
