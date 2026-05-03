#include "core.h"

#include <stdbool.h>
#include <stdio.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "auth.h"
#include "button.h"
#include "config.h"
#include "leds.h"
#include "rfid.h"
#include "state.h"
#include "ultrasonic.h"

static int64_t get_time_ms(void) { return esp_timer_get_time() / 1000; }

static bool time_elapsed(int64_t start_time_ms, int timeout_ms) {
  return get_time_ms() - start_time_ms >= timeout_ms;
}

static void set_state(System *system, State next_state) {
  system->current_state = next_state;
  system->state_started_at_ms = get_time_ms();

  printf("State: %d\n", next_state);
}

static void print_uid(const uint8_t *uid, int uid_len) {
  for (int i = 0; i < uid_len; i++) {
    printf("%02X", uid[i]);

    if (i < uid_len - 1) {
      printf(":");
    }
  }
}

static void reset_scan_data(System *system) {
  system->uid_len = 0;
  system->distance_cm = -1;
  rfid_clear_card();
}

static void system_turn_off(System *system) {
  system->is_enabled = false;
  reset_scan_data(system);

  leds_show_idle();
  set_state(system, State_Idle);
}

static void system_turn_on(System *system) {
  system->is_enabled = true;
  reset_scan_data(system);

  leds_show_waiting_for_ultrasonic();
  set_state(system, State_On);
}

static void start_cooldown(System *system) {
  system->cooldown_started_at_ms = get_time_ms();
  set_state(system, State_Cooldown);
}

static void clear_storage_and_reset(System *system) {
  printf("Auth: clearing enrolled card\n");

  if (auth_clear_enrolled_uid()) {
    printf("Auth: storage cleared\n");
  } else {
    printf("Auth: storage clear failed\n");
  }

  system->has_enrolled_card = false;

  system_turn_off(system);

  printf("Auth: first scanned card will enroll\n");
}

static bool handle_global_button(System *system) {
  static int press_count = 0;
  static int64_t first_press_time_ms = 0;

  bool handled_button = false;

  while (button_was_pressed()) {
    handled_button = true;

    int64_t now_ms = get_time_ms();

    if (press_count == 0 ||
        now_ms - first_press_time_ms > ClearStoragePressWindowMs) {
      press_count = 1;
      first_press_time_ms = now_ms;
    } else {
      press_count++;
    }

    printf("Button press count: %d\n", press_count);

    if (press_count >= ClearStoragePressCount) {
      press_count = 0;
      first_press_time_ms = 0;

      clear_storage_and_reset(system);
      return true;
    }

    if (system->current_state == State_Idle) {
      system_turn_on(system);
    } else {
      system_turn_off(system);
    }
  }

  return handled_button;
}

static void handle_idle(System *system) { (void)system; }

static void handle_on(System *system) {
  set_state(system, State_WaitingForUltrasonic);
}

static void handle_waiting_for_ultrasonic(System *system) {
  system->distance_cm = ultrasonic_get_distance_cm();

  if (system->distance_cm >= DetectionMinDistanceCm &&
      system->distance_cm <= DetectionMaxDistanceCm) {
    printf("Ultrasonic: received at %d cm\n", system->distance_cm);

    leds_show_waiting_for_rfid();
    set_state(system, State_UltrasonicReceived);
    return;
  }

  if (time_elapsed(system->state_started_at_ms, UltrasonicWaitMs)) {
    printf("Ultrasonic: timeout\n");

    leds_show_fail();
    start_cooldown(system);
  }
}

static void handle_ultrasonic_received(System *system) {
  set_state(system, State_WaitingForRfid);
}

static void handle_waiting_for_rfid(System *system) {
  if (!rfid_card_present()) {
    return;
  }

  system->uid_len = rfid_get_uid(system->uid, sizeof(system->uid));

  if (system->uid_len <= 0) {
    return;
  }

  printf("UID: ");
  print_uid(system->uid, system->uid_len);
  printf("\n");

  if (!auth_has_enrolled_uid()) {
    set_state(system, State_Enrolled);
    return;
  }

  set_state(system, State_Verify);
}

static void handle_enrolled(System *system) {
  if (!auth_enroll_uid(system->uid, system->uid_len)) {
    printf("Enrollment: FAIL\n");
    set_state(system, State_Fail);
    return;
  }

  system->has_enrolled_card = true;

  printf("Enrollment: SUCCESS\n");

  set_state(system, State_Verify);
}

static void handle_verify(System *system) {
  if (auth_uid_allowed(system->uid, system->uid_len)) {
    printf("Verification: SUCCESS\n");
    leds_show_success();
    reset_scan_data(system);
    set_state(system, State_Success);
    return;
  }

  printf("Verification: FAIL\n");
  leds_show_fail();
  reset_scan_data(system);
  set_state(system, State_Fail);
}

static void handle_success(System *system) {
  (void)system;

  /*
    RFID success is latched.
  */
}

static void handle_fail(System *system) {
  (void)system;

  /*
    RFID failure is latched.
    It stays red until the button is pressed.

  */
}

static void handle_cooldown(System *system) {
  if (!time_elapsed(system->cooldown_started_at_ms, FailedCooldownMs)) {
    return;
  }

  if (!system->is_enabled) {
    system_turn_off(system);
    return;
  }

  leds_show_waiting_for_ultrasonic();
  set_state(system, State_WaitingForUltrasonic);
}

static void system_update(System *system) {
  if (handle_global_button(system)) {
    return;
  }

  switch (system->current_state) {
  case State_Idle:
    handle_idle(system);
    break;

  case State_On:
    handle_on(system);
    break;

  case State_WaitingForUltrasonic:
    handle_waiting_for_ultrasonic(system);
    break;

  case State_UltrasonicReceived:
    handle_ultrasonic_received(system);
    break;

  case State_WaitingForRfid:
    handle_waiting_for_rfid(system);
    break;

  case State_Enrolled:
    handle_enrolled(system);
    break;

  case State_Verify:
    handle_verify(system);
    break;

  case State_Success:
    handle_success(system);
    break;

  case State_Fail:
    handle_fail(system);
    break;

  case State_Cooldown:
    handle_cooldown(system);
    break;
  }
}

void core_start(void) {
  System system = {
      .current_state = State_Idle,
      .is_enabled = false,
      .has_enrolled_card = false,
      .distance_cm = -1,
      .uid_len = 0,
      .state_started_at_ms = get_time_ms(),
      .cooldown_started_at_ms = 0,
  };

  auth_init();
  button_init();
  leds_init();
  ultrasonic_init();
  rfid_init();

  system.has_enrolled_card = auth_has_enrolled_uid();

  leds_show_idle();

  printf("System ready\n");

  if (system.has_enrolled_card) {
    printf("Auth: enrolled card found\n");
  } else {
    printf("Auth: first scanned card will enroll\n");
  }

  while (true) {
    system_update(&system);
    vTaskDelay(pdMS_TO_TICKS(LoopDelayMs));
  }
}
