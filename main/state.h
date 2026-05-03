#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  State_Idle,
  State_On,
  State_WaitingForUltrasonic,
  State_UltrasonicReceived,
  State_WaitingForRfid,
  State_Enrolled,
  State_Verify,
  State_Success,
  State_Fail,
  State_Cooldown,
} State;

typedef struct {
  State current_state;

  bool is_enabled;
  bool has_enrolled_card;

  int distance_cm;

  uint8_t uid[10];
  int uid_len;

  int64_t state_started_at_ms;
  int64_t cooldown_started_at_ms;
} System;
