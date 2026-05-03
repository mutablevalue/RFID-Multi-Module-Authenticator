#pragma once

#include <stdbool.h>
#include <stdint.h>

void rfid_init(void);
bool rfid_card_present(void);
int rfid_get_uid(uint8_t *buffer, int max_len);
void rfid_clear_card(void);
