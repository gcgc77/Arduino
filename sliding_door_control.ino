// --- Pin Definitions ---
const int PHOTOCELL_PIN_1 = 2; // Input from the first photocell
const int PHOTOCELL_PIN_2 = 3; // Input from the second photocell
const int RELAY_OPEN_PIN = 4;  // Output to the relay that opens the door
const int RELAY_CLOSE_PIN = 5; // Output to the relay that closes the door
const int MAIN_DOOR_SENSOR_PIN = 6; // Input from the main door's magnetic contact switch

// --- Sensor Logic Level ---
const int OBJECT_DETECTED = LOW;
const int MAIN_DOOR_IS_OPEN = LOW; // Magnetic switch pulls pin LOW when door opens

// --- Time Constants (in milliseconds) ---
const unsigned long DOOR_MOVE_DURATION = 10000;
const unsigned long DOOR_CLOSE_DELAY = 10000;
const unsigned long RELAY_PULSE_DURATION = 200;
const unsigned long OBJECT_DETECTION_DURATION = 500;

// --- Door State Machine ---
enum DoorState {
  DOOR_CLOSED,
  DOOR_OPENING,
  DOOR_OPEN,
  DOOR_CLOSING
};

// --- Global Variables ---
DoorState currentDoorState = DOOR_CLOSED;
unsigned long stateChangeTimestamp = 0;
unsigned long closeTimerTimestamp = 0;
unsigned long objectFirstDetectedTimestamp = 0;

// --- Non-blocking Pulse Management ---
bool openRelayPulseActive = false;
unsigned long openRelayPulseStart = 0;
bool closeRelayPulseActive = false;
unsigned long closeRelayPulseStart = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PHOTOCELL_PIN_1, INPUT_PULLUP);
  pinMode(PHOTOCELL_PIN_2, INPUT_PULLUP);
  pinMode(MAIN_DOOR_SENSOR_PIN, INPUT_PULLUP); // Enable pull-up for the switch
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

  // --- High-Priority Master Door Check ---
  if (digitalRead(MAIN_DOOR_SENSOR_PIN) == MAIN_DOOR_IS_OPEN) {
    // If the main door is open, the sliding door MUST be closed.
    // This overrides all other logic.
    if (currentDoorState == DOOR_OPEN || currentDoorState == DOOR_OPENING) {
      Serial.println("Main door opened! Overriding and closing sliding door.");
      changeState(DOOR_CLOSING);
      triggerRelay(RELAY_CLOSE_PIN);
    }
    // By returning here, we skip the entire photocell logic below.
    return;
  }

  // --- Normal Operation (runs only if main door is closed) ---
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
      // We add a check here to ensure the safety override for photocells
      // does NOT run if the main door is open.
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
