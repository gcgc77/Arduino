#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc; // Create an RTC object

// --- Pin Definitions ---
const int PHOTOCELL_PIN_1 = 2;
const int PHOTOCELL_PIN_2 = 3;
const int RELAY_OPEN_PIN = 4;
const int RELAY_CLOSE_PIN = 5;
const int MAIN_DOOR_SENSOR_PIN = 6;

// --- Sensor Logic Level ---
const int OBJECT_DETECTED = LOW;
const int MAIN_DOOR_IS_OPEN = HIGH;

// --- Time Constants (in milliseconds) ---
const unsigned long DOOR_MOVE_DURATION = 10000;
const unsigned long DOOR_CLOSE_DELAY = 10000;
const unsigned long RELAY_PULSE_DURATION = 200;
const unsigned long OBJECT_DETECTION_DURATION = 250;

// --- Night Lock Configuration ---
const int NIGHT_LOCK_START_HOUR = 22; // 10 PM
const int NIGHT_LOCK_END_HOUR = 5;    // 5 AM

// --- Door State Machine & Global Variables ---
enum DoorState { DOOR_CLOSED, DOOR_OPENING, DOOR_OPEN, DOOR_CLOSING };
DoorState currentDoorState = DOOR_CLOSED;
unsigned long stateChangeTimestamp = 0;
unsigned long closeTimerTimestamp = 0;
unsigned long objectFirstDetectedTimestamp = 0;
bool openRelayPulseActive = false;
unsigned long openRelayPulseStart = 0;
bool closeRelayPulseActive = false;
unsigned long closeRelayPulseStart = 0;


void setup() {
  Serial.begin(9600);

  // --- Initialize RTC ---
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    Serial.flush();
    abort();
  }
  // The following line should be run ONCE to set the time.
  // After running it once, comment it out and re-upload.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // --- Pin Setup ---
  pinMode(PHOTOCELL_PIN_1, INPUT_PULLUP);
  pinMode(PHOTOCELL_PIN_2, INPUT_PULLUP);
  pinMode(MAIN_DOOR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RELAY_OPEN_PIN, OUTPUT);
  pinMode(RELAY_CLOSE_PIN, OUTPUT);

  digitalWrite(RELAY_OPEN_PIN, HIGH);
  digitalWrite(RELAY_CLOSE_PIN, HIGH);

  Serial.println("Sliding Door Controller Initialized.");
}

void loop() {
  // --- Manage active relay pulses ---
  if (openRelayPulseActive && (millis() - openRelayPulseStart >= RELAY_PULSE_DURATION)) {
    digitalWrite(RELAY_OPEN_PIN, HIGH);
    openRelayPulseActive = false;
  }
  if (closeRelayPulseActive && (millis() - closeRelayPulseStart >= RELAY_PULSE_DURATION)) {
    digitalWrite(RELAY_CLOSE_PIN, HIGH);
    closeRelayPulseActive = false;
  }

  // --- HIGHEST Priority: Night Lock Check ---
  if (isNightLockActive()) {
    if (currentDoorState == DOOR_OPEN || currentDoorState == DOOR_OPENING) {
      Serial.println("Night lock active! Closing door.");
      changeState(DOOR_CLOSING);
      triggerRelay(RELAY_CLOSE_PIN);
    }
    return; // Ignore all other sensors
  }

  // --- High-Priority Master Door Check ---
  if (digitalRead(MAIN_DOOR_SENSOR_PIN) == MAIN_DOOR_IS_OPEN) {
    if (currentDoorState == DOOR_OPEN || currentDoorState == DOOR_OPENING) {
      Serial.println("Main door opened! Closing sliding door.");
      changeState(DOOR_CLOSING);
      triggerRelay(RELAY_CLOSE_PIN);
    }
    return;
  }

  // --- Normal Operation ---
  bool objectDetected = isObjectDetected();
  switch (currentDoorState) {
    case DOOR_CLOSED:
      if (objectDetected) {
        if (objectFirstDetectedTimestamp == 0) {
          objectFirstDetectedTimestamp = millis();
        }
      } else {
        objectFirstDetectedTimestamp = 0;
      }
      if (objectFirstDetectedTimestamp != 0 && (millis() - objectFirstDetectedTimestamp >= OBJECT_DETECTION_DURATION)) {
        changeState(DOOR_OPENING);
        triggerRelay(RELAY_OPEN_PIN);
        objectFirstDetectedTimestamp = 0;
      }
      break;
    case DOOR_OPENING:
      if (millis() - stateChangeTimestamp >= DOOR_MOVE_DURATION) {
        changeState(DOOR_OPEN);
      }
      break;
    case DOOR_OPEN:
      if (objectDetected) {
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
      if (objectDetected) {
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
bool isNightLockActive() {
  DateTime now = rtc.now();
  int currentHour = now.hour();
  return (currentHour >= NIGHT_LOCK_START_HOUR || currentHour < NIGHT_LOCK_END_HOUR);
}

bool isObjectDetected() {
  return (digitalRead(PHOTOCELL_PIN_1) == OBJECT_DETECTED || digitalRead(PHOTOCELL_PIN_2) == OBJECT_DETECTED);
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
