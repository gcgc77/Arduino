# User Manual: Smart Sliding Door Controller

## 1. Overview

This project is an advanced controller for an automated sliding door, built using an Arduino. It uses multiple sensors to make intelligent decisions about when to open and close, and includes several layers of safety and security features.

The system is designed to control a sliding door that is embedded within a larger, manually operated door, and includes logic to handle this specific scenario.

## 2. Features

*   **Automatic Opening:** The door opens automatically when an object is detected by one of two photocell sensors.
*   **Configurable Detection Time:** To prevent false triggers, the door only opens after an object has been detected for a configurable amount of time (default 250ms). Each photocell has its own independent timer.
*   **Timed Auto-Close:** The door automatically closes after being open for a set period if no objects are detected.
*   **Safety Interrupt:** If an object is detected by the photocells while the door is closing, it will immediately stop and reopen.
*   **Main Door Override:** A sensor on the larger, main door will force the sliding door to close immediately if the main door is opened. This has priority over the photocells.
*   **Night Lock Security:** The controller uses a Real-Time Clock (RTC) to implement a time-based lock. During a configurable time window (default 10 PM to 5 AM), the door is forced closed and will not open, providing enhanced security. This is the highest priority feature.
*   **Pulse-Based Relay Control:** The code sends short pulses to the relays instead of a continuous signal, making it compatible with smart relays (like Shelly) or standard relay modules.
*   **Support for Active-LOW Relays:** The logic is configured to work with common Active-LOW relay modules.

## 3. Required Hardware

*   **1x Arduino Board** (e.g., Arduino Uno, Nano)
*   **2x E18-D80NK Photocells** (or similar 5V digital IR proximity sensors)
*   **1x 2-Channel 5V Relay Module** (must be Active-LOW)
*   **1x Magnetic Contact Switch** (for the main door sensor, "Normally Closed" type recommended for current code)
*   **1x DS3231 Real-Time Clock (RTC) Module**
*   **Jumper Wires**
*   **1x Breadboard** (recommended for easily sharing power connections)

## 4. Wiring Instructions

It is recommended to use a breadboard to distribute power (`5V`) and ground (`GND`) to the multiple components that need it.

| Component                 | Pin on Component | Connect to Arduino Pin | Notes                               |
| ------------------------- | ---------------- | ---------------------- | ----------------------------------- |
| **Photocell 1**           | `VCC`            | `5V`                   | Connect to the 5V power rail        |
|                           | `GND`            | `GND`                  | Connect to the common ground rail   |
|                           | `OUT`            | `Digital Pin 2`        |                                     |
| **Photocell 2**           | `VCC`            | `5V`                   | Connect to the 5V power rail        |
|                           | `GND`            | `GND`                  | Connect to the common ground rail   |
|                           | `OUT`            | `Digital Pin 3`        |                                     |
| **2-Channel Relay Module**| `VCC`            | `5V`                   | Connect to the 5V power rail        |
|                           | `GND`            | `GND`                  | Connect to the common ground rail   |
|                           | `IN1`            | `Digital Pin 4`        | Controls the "Open" relay           |
|                           | `IN2`            | `Digital Pin 5`        | Controls the "Close" relay          |
| **Main Door Sensor**      | `Wire 1`         | `Digital Pin 6`        | Polarity does not matter            |
| (Magnetic Switch)         | `Wire 2`         | `GND`                  | Connect to the common ground rail   |
| **DS3231 RTC Module**     | `VCC`            | `5V`                   | Connect to the 5V power rail        |
|                           | `GND`            | `GND`                  | Connect to the common ground rail   |
|                           | `SDA`            | `Pin A4`               | I2C Data Pin (on Uno/Nano)          |
|                           | `SCL`            | `Pin A5`               | I2C Clock Pin (on Uno/Nano)         |

## 5. Software Setup

### 5.1. Install Required Library

This project requires the **"RTClib"** library by Adafruit to communicate with the DS3231 RTC module.
1.  In the Arduino IDE, go to **Sketch > Include Library > Manage Libraries...**
2.  In the search box, type `RTClib`.
3.  Find the library by **Adafruit** and click the "Install" button.

### 5.2. Set the Time on the RTC

The RTC module has a battery, but it doesn't know the current time when you first get it. You must set it once.
1.  Open the `sliding_door_control.ino` sketch.
2.  Find the `setup()` function.
3.  Locate this line: `// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));`
4.  **Uncomment the line** by removing the `//` at the beginning.
5.  **Upload the code** to your Arduino. This sets the RTC's time to the date and time on your computer at the moment of compilation.
6.  **IMMEDIATELY re-comment the line** by adding the `//` back.
7.  **Upload the code a second time.** This is critical. If you skip this step, the time will be reset every time the Arduino powers on.

## 6. Code Configuration

The main sketch file has several constants at the top that you can change to easily configure the door's behavior without altering the main logic.

| Constant Name                      | Default Value | Description                                                                 |
| ---------------------------------- | ------------- | --------------------------------------------------------------------------- |
| `MAIN_DOOR_IS_OPEN`                | `HIGH`        | Set to `HIGH` for Normally Closed (NC) switches, `LOW` for Normally Open (NO). |
| `DOOR_MOVE_DURATION`               | `10000`       | The time (in ms) it takes for the door to fully open or close.                |
| `DOOR_CLOSE_DELAY`                 | `14000`       | The time (in ms) to wait before closing the door after an object is gone.     |
| `PHOTOCELL_1_DETECTION_DURATION`   | `250`         | The time (in ms) an object must be seen by photocell 1 to trigger opening.  |
| `PHOTOCELL_2_DETECTION_DURATION`   | `250`         | The time (in ms) an object must be seen by photocell 2 to trigger opening.  |
| `NIGHT_LOCK_START_HOUR`            | `22`          | The hour (0-23) when the nighttime lock engages. (10 PM)                      |
| `NIGHT_LOCK_END_HOUR`              | `5`           | The hour (0-23) when the nighttime lock disengages. (5 AM)                    |
