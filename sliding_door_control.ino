// --- Pin Definitions ---
const int PHOTOCELL_PIN_1 = 2;
const int PHOTOCELL_PIN_2 = 3;
const int PHOTOCELL_PIN_3 = 8; // New long-range photocell
const int RELAY_OPEN_PIN = 4;
const int RELAY_CLOSE_PIN = 5;
const int MAIN_DOOR_SENSOR_PIN = 6;

// --- Sensor Logic Level ---
const int OBJECT_DETECTED = LOW;
const int MAIN_DOOR_IS_OPEN = HIGH;

// --- Time Constants (in milliseconds) ---
const unsigned long DOOR_MOVE_DURATION = 10000;
const unsigned long DOOR_CLOSE_DELAY = 25000;
const unsigned long RELAY_PULSE_DURATION = 200;
const unsigned long PHOTOCELL_1_DETECTION_DURATION = 200;
const unsigned long PHOTOCELL_2_DETECTION_DURATION = 200;
const unsigned long PHOTOCELL_3_DETECTION_DURATION = 200; // Timer for new sensor

// --- Door State Machine & Global Variables ---
enum DoorState { DOOR_CLOSED, DOOR_OPENING, DOOR_OPEN, DOOR_CLOSING };
DoorState currentDoorState = DOOR_CLOSED;
unsigned long stateChangeTimestamp = 0;
unsigned long closeTimerTimestamp = 0;
unsigned long photocell1FirstDetectedTimestamp = 0;
unsigned long photocell2FirstDetectedTimestamp = 0;
unsigned long photocell3FirstDetectedTimestamp = 0; // Timer for new sensor
bool isReversing = false;
unsigned long reversalEndTime = 0;

// --- Non-blocking Pulse Management ---
bool openRelayPulseActive = false;
unsigned long openRelayPulseStart = 0;
bool closeRelayPulseActive = false;
unsigned long closeRelayPulseStart = 0;


void setup() {
  Serial.begin(9600);
  pinMode(PHOTOCELL_PIN_1, INPUT_PULLUP);
  pinMode(PHOTOCELL_PIN_2, INPUT_PULLUP);
  pinMode(PHOTOCELL_PIN_3, INPUT_PULLUP); // New sensor
  pinMode(MAIN_DOOR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RELAY_OPEN_PIN, OUTPUT);
  pinMode(RELAY_CLOSE_PIN, OUTPUT);
  digitalWrite(RELAY_OPEN_PIN, HIGH);
  digitalWrite(RELAY_CLOSE_PIN, HIGH);
  Serial.println("Sliding Door Controller Initialized.");
}

void loop() {
  // Pulse management logic
  if (openRelayPulseActive && (millis() - openRelayPulseStart >= RELAY_PULSE_DURATION)) {
    digitalWrite(RELAY_OPEN_PIN, HIGH);
    openRelayPulseActive = false;
  }
  if (closeRelayPulseActive && (millis() - closeRelayPulseStart >= RELAY_PULSE_DURATION)) {
    digitalWrite(RELAY_CLOSE_PIN, HIGH);
    closeRelayPulseActive = false;
  }

  // High-Priority: Master Door Check
  if (digitalRead(MAIN_DOOR_SENSOR_PIN) == MAIN_DOOR_IS_OPEN) {
    if (currentDoorState == DOOR_OPEN || currentDoorState == DOOR_OPENING) {
      changeState(DOOR_CLOSING);
      triggerRelay(RELAY_CLOSE_PIN);
    }
    return;
  }

  // Normal Operation
  switch (currentDoorState) {
    case DOOR_CLOSED:
      {
        bool photocell1_detected = (digitalRead(PHOTOCELL_PIN_1) == OBJECT_DETECTED);
        bool photocell2_detected = (digitalRead(PHOTOCELL_PIN_2) == OBJECT_DETECTED);
        bool photocell3_detected = (digitalRead(PHOTOCELL_PIN_3) == OBJECT_DETECTED);

        // Manage Timers
        if (photocell1_detected) {
          if (photocell1FirstDetectedTimestamp == 0) photocell1FirstDetectedTimestamp = millis();
        } else {
          photocell1FirstDetectedTimestamp = 0;
        }
        if (photocell2_detected) {
          if (photocell2FirstDetectedTimestamp == 0) photocell2FirstDetectedTimestamp = millis();
        } else {
          photocell2FirstDetectedTimestamp = 0;
        }
        if (photocell3_detected) {
          if (photocell3FirstDetectedTimestamp == 0) photocell3FirstDetectedTimestamp = millis();
        } else {
          photocell3FirstDetectedTimestamp = 0;
        }

        // Check if any timer has met its condition
        bool triggerOpen = false;
        if (photocell1FirstDetectedTimestamp != 0 && (millis() - photocell1FirstDetectedTimestamp >= PHOTOCELL_1_DETECTION_DURATION)) {
          triggerOpen = true;
        }
        if (!triggerOpen && photocell2FirstDetectedTimestamp != 0 && (millis() - photocell2FirstDetectedTimestamp >= PHOTOCELL_2_DETECTION_DURATION)) {
          triggerOpen = true;
        }
        if (!triggerOpen && photocell3FirstDetectedTimestamp != 0 && (millis() - photocell3FirstDetectedTimestamp >= PHOTOCELL_3_DETECTION_DURATION)) {
          triggerOpen = true;
        }

        if (triggerOpen) {
          isReversing = false;
          changeState(DOOR_OPENING);
          triggerRelay(RELAY_OPEN_PIN);
          photocell1FirstDetectedTimestamp = 0;
          photocell2FirstDetectedTimestamp = 0;
          photocell3FirstDetectedTimestamp = 0;
        }
      }
      break;
    case DOOR_OPENING:
      if (isReversing) {
        if (millis() >= reversalEndTime) {
          triggerRelay(RELAY_OPEN_PIN);
          isReversing = false;
          changeState(DOOR_OPEN);
        }
      } else {
        if (millis() - stateChangeTimestamp >= DOOR_MOVE_DURATION) {
          triggerRelay(RELAY_OPEN_PIN);
          changeState(DOOR_OPEN);
        }
      }
      break;
    case DOOR_OPEN:
      if (isObjectDetected()) {
        closeTimerTimestamp = 0;
      } else {
        if (closeTimerTimestamp == 0) {
          closeTimerTimestamp = millis();
        }
        if (millis() - closeTimerTimestamp >= DOOR_CLOSE_DELAY) {
          changeState(DOOR_CLOSING);
          triggerRelay(RELAY_CLOSE_PIN);
          closeTimerTimestamp = 0;
        }
      }
      break;
    case DOOR_CLOSING:
      if (isObjectDetected()) {
        triggerRelay(RELAY_CLOSE_PIN);
        unsigned long timeSpentClosing = millis() - stateChangeTimestamp;
        reversalEndTime = millis() + timeSpentClosing;
        isReversing = true;
        changeState(DOOR_OPENING);
        triggerRelay(RELAY_OPEN_PIN);
      } else {
        if (millis() - stateChangeTimestamp >= DOOR_MOVE_DURATION) {
          changeState(DOOR_CLOSED);
        }
      }
      break;
  }
}

// --- Helper Functions ---
bool isObjectDetected() {
  // Check all three sensors
  return (digitalRead(PHOTOCELL_PIN_1) == OBJECT_DETECTED ||
          digitalRead(PHOTOCELL_PIN_2) == OBJECT_DETECTED ||
          digitalRead(PHOTOCELL_PIN_3) == OBJECT_DETECTED);
}
void triggerRelay(int pin) {
  if (pin == RELAY_OPEN_PIN && !openRelayPulseActive) {
    digitalWrite(RELAY_OPEN_PIN, LOW);
    openRelayPulseActive = true;
    openRelayPulseStart = millis();
  } else if (pin == RELAY_CLOSE_PIN && !closeRelayPulseActive) {
    digitalWrite(RELAY_CLOSE_PIN, LOW);
    closeRelayPulseActive = true;
    closeRelayPulseStart = millis();
  }
}
void changeState(DoorState newState) {
  currentDoorState = newState;
  stateChangeTimestamp = millis();
  Serial.print("State changed to: ");
  switch (newState) {
    case DOOR_CLOSED: Serial.println("DOOR_CLOSED"); break;
    case DOOR_OPENING: Serial.println("DOOR_OPENING"); break;
    case DOOR_OPEN: Serial.println("DOOR_OPEN"); break;
    case DOOR_CLOSING: Serial.println("DOOR_CLOSING"); break;
  }
}
