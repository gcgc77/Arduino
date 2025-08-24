# User Manual: Smart Sliding Door Controller

## 1. Overview

This project is an advanced controller for an automated sliding door, built using an Arduino. It uses multiple sensors to make intelligent decisions about when to open and close, and includes several layers of safety features.

The system is designed to control a sliding door that is embedded within a larger, manually operated door, and includes logic to handle this specific scenario.

## 2. Features

*   **Multi-Sensor Opening:** The door opens automatically when an object is detected by any of three photocell sensors.
*   **Configurable Detection Time:** To prevent false triggers, the door only opens after an object has been detected for a configurable amount of time. Each photocell has its own independent timer.
*   **Timed Auto-Close:** The door automatically closes after being open for a set period if no objects are detected.
*   **Proportional Safety Reversal:** If an object is detected while the door is closing, it will immediately reverse and open for the same amount of time that it was closing, ensuring a fast and efficient reversal.
*   **Main Door Override:** A sensor on the larger, main door will force the sliding door to close immediately if the main door is opened.
*   **Pulse-Based Relay Control:** The code sends short pulses to the relays, making it compatible with smart relays or standard relay modules.
*   **Support for Active-LOW Relays:** The logic is configured to work with common Active-LOW relay modules.
*   **Precise Motor Control:** Sends a second pulse at the end of the opening sequence to reliably stop the motor. The closing sequence is allowed to run to completion, relying on the motor's internal end-stop.

## 3. Required Hardware

*   **1x Arduino Board** (e.g., Arduino Uno, Nano)
*   **2x E18-D80NK Photocells** (5V, short-range)
*   **1x E3F-DS200C3 Photocell** (10-30V, long-range, NPN NO)
*   **1x 2-Channel 5V Relay Module** (must be Active-LOW)
*   **1x Magnetic Contact Switch** (for the main door sensor)
*   **Jumper Wires**
*   **1x Breadboard** (recommended for easily sharing power connections)

## 4. Wiring Instructions

It is recommended to use a breadboard to distribute power (`5V`, `Vin`) and ground (`GND`) to the multiple components that need it. **This setup assumes you are powering the Arduino with a 12V DC power supply via the round barrel jack.**

| Component                 | Pin on Component | Connect to Arduino Pin | Notes                               |
| ------------------------- | ---------------- | ---------------------- | ----------------------------------- |
| **Photocell 1 (Short)**   | `VCC`            | `5V`                   | Connect to the 5V power rail        |
|                           | `GND`            | `GND`                  | Connect to the common ground rail   |
|                           | `OUT`            | `Digital Pin 2`        |                                     |
| **Photocell 2 (Short)**   | `VCC`            | `5V`                   | Connect to the 5V power rail        |
|                           | `GND`            | `GND`                  | Connect to the common ground rail   |
|                           | `OUT`            | `Digital Pin 3`        |                                     |
| **Photocell 3 (Long)**    | `Brown (V+)`     | `Vin`                  | **CRITICAL:** Use Vin for 12V power |
|                           | `Blue (GND)`     | `GND`                  | Connect to the common ground rail   |
|                           | `Black (OUT)`    | `Digital Pin 8`        |                                     |
| **2-Channel Relay Module**| `VCC`            | `5V`                   | Connect to the 5V power rail        |
|                           | `GND`            | `GND`                  | Connect to the common ground rail   |
|                           | `IN1`            | `Digital Pin 4`        | Controls the "Open" relay           |
|                           | `IN2`            | `Digital Pin 5`        | Controls the "Close" relay          |
| **Main Door Sensor**      | `Wire 1`         | `Digital Pin 6`        | Polarity does not matter            |
| (Magnetic Switch)         | `Wire 2`         | `GND`                  | Connect to the common ground rail   |

## 5. Code Configuration

The main sketch file has several constants at the top that you can change to easily configure the door's behavior without altering the main logic.

| Constant Name                      | Default Value | Description                                                                 |
| ---------------------------------- | ------------- | --------------------------------------------------------------------------- |
| `MAIN_DOOR_IS_OPEN`                | `HIGH`        | Set to `HIGH` for Normally Closed (NC) switches, `LOW` for Normally Open (NO). |
| `DOOR_MOVE_DURATION`               | `10000`       | The time (in ms) it takes for the door to fully open or close.                |
| `DOOR_CLOSE_DELAY`                 | `25000`       | The time (in ms) to wait before closing the door after an object is gone.     |
| `PHOTOCELL_1_DETECTION_DURATION`   | `200`         | The time (in ms) an object must be seen by photocell 1 to trigger opening.  |
| `PHOTOCELL_2_DETECTION_DURATION`   | `200`         | The time (in ms) an object must be seen by photocell 2 to trigger opening.  |
| `PHOTOCELL_3_DETECTION_DURATION`   | `200`         | The time (in ms) an object must be seen by photocell 3 to trigger opening.  |
