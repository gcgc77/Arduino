# User Manual: Smart Sliding Door Controller

## 1. Overview

This project is an advanced controller for an automated sliding door, built using an Arduino. It uses multiple sensors to make intelligent decisions about when to open and close, and includes several layers of safety and security features.

The system is designed to control a sliding door that is embedded within a larger, manually operated door, and includes logic to handle this specific scenario.

## 2. System Architecture

This controller uses a two-stage relay system for robust operation:

1.  **Arduino & Relay Module:** The Arduino runs the primary logic. It sends simple open/close/stop commands to a standard 2-channel relay module. This module acts as the low-voltage "brain" of the operation.
2.  **Shelly PM Smart Relay:** The outputs of the Arduino's relay module are connected to the switch inputs of a Shelly PM smart relay. The Shelly relay is responsible for handling the high-voltage power required to physically drive the door motor.

This architecture is flexible and safe, as the Arduino never directly handles high-voltage power.

## 3. Features

*   **Multi-Sensor Opening:** The door opens automatically when an object is detected by any of three photocell sensors.
*   **Configurable Detection Time:** To prevent false triggers, the door only opens after an object has been detected for a configurable amount of time. Each photocell has its own independent timer.
*   **Timed Auto-Close:** The door automatically closes after being open for a set period if no objects are detected.
*   **Proportional Safety Reversal:** If an object is detected while the door is closing, it will immediately reverse and open for the same amount of time that it was closing, ensuring a fast and efficient reversal.
*   **Main Door Override:** A sensor on the larger, main door will force the sliding door to close immediately if the main door is opened.
*   **Pulse-Based Relay Control:** The code sends short pulses to the relay module, which then trigger the Shelly PM.
*   **Precise Motor Control:** Sends a second pulse at the end of the opening sequence to reliably stop the motor. The closing sequence is allowed to run to completion, relying on the motor's internal end-stop.

## 4. Required Hardware

*   **1x Arduino Board** (e.g., Arduino Uno, Nano)
*   **1x Shelly PM Smart Relay** (or similar smart relay that controls the door motor)
*   **2x E18-D80NK Photocells** (5V, short-range)
*   **1x E3F-DS200C3 Photocell** (10-30V, long-range, NPN NO)
*   **1x 2-Channel 5V Relay Module** (must be Active-LOW)
*   **1x Magnetic Contact Switch** (for the main door sensor)
*   **Jumper Wires**
*   **1x Breadboard** (recommended for easily sharing power connections)

## 5. Wiring Instructions

(See previous sections for detailed wiring table)

## 6. Detailed Logic Explanation

### The `DOOR_MOVE_DURATION` Timer

The `DOOR_MOVE_DURATION` constant (default: 10,000ms) is one of the most important variables in the sketch, but it does **not** physically limit how long the motor runs. Instead, it is the Arduino's internal "guess" for how long a full open or close cycle should take.

Its primary purposes are:
1.  **To Time a Normal Opening:** When the door opens from a fully closed position, the Arduino uses this duration to know when the door *should* be fully open. At the end of this time, it sends the "stop" pulse.
2.  **To Enable Proportional Reversal:** This is its most critical function. When an object is detected during a closing cycle, the Arduino calculates `timeSpentClosing`. It then uses this value to determine the `reversalEndTime`. This "smart" reversal is only possible because the Arduino is timing the movement against the `DOOR_MOVE_DURATION`.
3.  **To Time a Normal Closing:** The Arduino uses this duration to know when a normal closing sequence is complete.

**Synchronization with Shelly:** As discovered during troubleshooting, it is critical that the Shelly PM's own **"Max Opening/Closing Time"** safety timer is set to be **longer** than the Arduino's `DOOR_MOVE_DURATION`. This ensures that the Arduino is always the one in control of the timing and can successfully send its "stop" pulse before the Shelly times out and stops the motor on its own.

## 7. Code Configuration

(See previous sections for detailed configuration table)
