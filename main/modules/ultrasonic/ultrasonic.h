#pragma once

#include <stdbool.h>

void ultrasonic_init(void);
int ultrasonic_get_distance_cm(void);
bool ultrasonic_detected_object(int max_distance_cm);
