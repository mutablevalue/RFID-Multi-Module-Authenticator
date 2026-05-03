# RFID Multi Module Authenticator

This project is an ESP32-based RFID access verification system that uses an ultrasonic sensor, an RC522 RFID reader, a push button, and status LEDs to control a simple verification workflow.

The system waits for a button press to activate. Once active, it checks whether an object is within the required ultrasonic distance range. If the object is detected, the system waits for an RFID card scan. The first scanned card can be enrolled and saved to storage, and later scans are compared against that saved card. A successful RFID verification leaves the system in a green success state, while a failed verification leaves it in a red failure state until the button resets the system.

## Project Roles

This project was completed as a two-person effort.

My partner selected the GPIO pin assignments, planned the wiring layout, and physically wired the ESP32, LEDs, ultrasonic sensor, RFID reader, and button. Their work handled the hardware side of the project, including connecting the components to the correct pins and making sure the circuit could support the intended inputs and outputs.

I programmed the embedded system logic. This included the state machine, RFID enrollment and verification behavior, ultrasonic detection flow, button-based reset behavior, LED status output, and persistent card storage using NVS.

## Hardware Used

- ESP32 development board
- RC522 RFID reader
- HC-SR04 ultrasonic sensor
- Push button
- Status LEDs
- Resistors for LEDs and button/voltage protection as needed
- Jumper wires and breadboard

## GPIO Pin Assignments

| Component | Function | GPIO Pin |
|---|---:|---:|
| Button | Input | GPIO 27 |
| Button LED | Output | GPIO 14 |
| Ultrasonic Sensor | Echo | GPIO 2 |
| Ultrasonic Sensor | Trigger | GPIO 4 |
| Ultrasonic LED | Red | GPIO 33 |
| Ultrasonic LED | Yellow | GPIO 25 |
| Ultrasonic LED | Green | GPIO 26 |
| RFID RC522 | Reset | GPIO 22 |
| RFID RC522 | MISO | GPIO 19 |
| RFID RC522 | MOSI | GPIO 23 |
| RFID RC522 | CS / SDA / SS | GPIO 5 |
| RFID RC522 | SCK | GPIO 18 |
| RFID LED | Red | GPIO 32 |
| RFID LED | Yellow | GPIO 13 |
| RFID LED | Green | GPIO 21 |

## System Behavior

### Idle State

When the system first starts, it remains idle. The button LED is off, and the system waits for the user to press the button.

### Activation

When the button is pressed, the system turns on and begins waiting for ultrasonic detection. The ultrasonic LEDs show that the system is actively waiting for an object to be detected.

### Ultrasonic Detection

The ultrasonic sensor checks whether an object is within the configured detection range. If no valid object is detected before the timeout, the system briefly enters a cooldown and then tries again. Ultrasonic timeout does not count as an RFID failure.

### RFID Verification

After a valid ultrasonic detection, the system waits for an RFID card scan.

- If no enrolled card exists, the first scanned card is saved as the enrolled card.
- If an enrolled card already exists, the scanned card is compared against the stored card.
- If the scanned UID matches the stored UID, verification succeeds.
- If the scanned UID does not match, verification fails.

### Success State

On successful RFID verification, the system displays the green success LEDs and remains in that state until the button is pressed.

### Failure State

On failed RFID verification, the system displays the red failure LEDs and remains in that state until the button is pressed.

### Storage Reset

Pressing the button rapidly four times clears the stored RFID card from non-volatile storage. After this reset, the system returns to first-run behavior, meaning the next valid RFID card can be enrolled as the new authorized card.

## Software Structure

The project is organized into separate modules for readability and maintainability.

| Module | Purpose |
|---|---|
| `core` | Main state machine and system workflow |
| `auth` | RFID enrollment, verification, and NVS storage |
| `button` | Button input handling and reset detection |
| `leds` | LED output control for system status |
| `rfid` | RC522 RFID scanner setup and UID reading |
| `ultrasonic` | HC-SR04 distance measurement |
| `config` | GPIO pins, timing values, and detection constants |

## State Flow

```text
Idle
  -> On
  -> WaitingForUltrasonic
  -> UltrasonicReceived
  -> WaitingForRfid
  -> Enrolled, if no stored card exists
  -> Verify
  -> Success or Fail
```

Ultrasonic timeout uses a cooldown cycle and then returns to ultrasonic detection. RFID success and RFID failure are latched states that require a button press to reset.

## Build and Flash

This project is intended to be built with ESP-IDF.

```bash
idf.py build
idf.py flash monitor
```

## Notes

All of it is powered using 3.3v, which can cause issues of strength in the ultrasonic sensor. You can use 5v with a voltage divider on the RC522 RFID Module if wanted.
