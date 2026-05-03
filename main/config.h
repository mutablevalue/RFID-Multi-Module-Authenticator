#pragma once

// Button
static const int ButtonInputGpio = 27;
static const int ButtonLedGpio = 14;

// Ultrasonic 
static const int UltrasonicEchoGpio = 2;
static const int UltrasonicTrigGpio = 4;

static const int UltrasonicRedLedGpio = 33;
static const int UltrasonicYellowLedGpio = 25;
static const int UltrasonicGreenLedGpio = 26;

// RFID
static const int RfidRstGpio = 22;
static const int RfidMisoGpio = 19;
static const int RfidMosiGpio = 23;
static const int RfidCsGpio = 5;
static const int RfidSckGpio = 18;

static const int RfidRedLedGpio = 32;
static const int RfidYellowLedGpio = 13;
static const int RfidGreenLedGpio = 21;

// Timing
static const int UltrasonicWaitMs = 5000;
static const int FailedCooldownMs = 5000;
static const int UltrasonicCooldownMs = 5000;
static const int LoopDelayMs = 50;

// Detection
static const int DetectionMinDistanceCm = 10;
static const int DetectionMaxDistanceCm = 20;

static const int ClearStoragePressCount = 4;
static const int ClearStoragePressWindowMs = 1500;
