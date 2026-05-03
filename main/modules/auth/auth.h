#pragma once

#include <stdbool.h>
#include <stdint.h>

void auth_init(void);

bool auth_has_enrolled_uid(void);
bool auth_enroll_uid(const uint8_t *uid, int uid_len);
bool auth_uid_allowed(const uint8_t *uid, int uid_len);
bool auth_clear_enrolled_uid(void);
