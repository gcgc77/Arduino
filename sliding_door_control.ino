// --- Pin Definitions ---
const int PHOTOCELL_PIN_1 = 2; // Input from the first photocell
const int PHOTOCELL_PIN_2 = 3; // Input from the second photocell
const int RELAY_OPEN_PIN = 4;  // Output to the relay that opens the door
const int RELAY_CLOSE_PIN = 5; // Output to the relay that closes the door

// --- Sensor Logic Level ---
const int OBJECT_DETECTED = LOW;

// --- Time Constants (in milliseconds) ---
const unsigned long DOOR_MOVE_DURATION = 10000; // 10 seconds for the door to fully open or close
const unsigned long DOOR_CLOSE_DELAY = 10000;   // 10 seconds to wait before closing the door
const unsigned long RELAY_PULSE_DURATION = 200; // 200ms pulse for Shelly relay trigger

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

// --- Non-blocking Pulse Management ---
bool openRelayPulseActive = false;
unsigned long openRelayPulseStart = 0;
bool closeRelayPulseActive = false;
unsigned long closeRelayPulseStart = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PHOTOCELL_PIN_1, INPUT_PULLUP);
  pinMode(PHOTOCELL_PIN_2, INPUT_PULLUP);
  pinMode(RELAY_OPEN_PIN, OUTPUT);
  pinMode(RELAY_CLOSE_PIN, OUTPUT);
  digitalWrite(RELAY_OPEN_PIN, LOW);
  digitalWrite(RELAY_CLOSE_PIN, LOW);
  Serial.println("Sliding Door Controller Initialized (Trigger Mode).");
  Serial.println("Current State: DOOR_CLOSED");
}

void loop() {
  // --- Manage active relay pulses (non-blocking) ---
  if (openRelayPulseActive && (millis() - openRelayPulseStart >= RELAY_PULSE_DURATION)) {
    digitalWrite(RELAY_OPEN_PIN, LOW);
    openRelayPulseActive = false;
  }
  if (closeRelayPulseActive && (millis() - closeRelayPulseStart >= RELAY_PULSE_DURATION)) {
    digitalWrite(RELAY_CLOSE_PIN, LOW);
    closeRelayPulseActive = false;
  }

  bool objectDetected = isObjectDetected();

  switch (currentDoorState) {
    case DOOR_CLOSED:
      if (objectDetected) {
        changeState(DOOR_OPENING);
        triggerRelay(RELAY_OPEN_PIN); // Send a pulse to open
      }
      break;

    case DOOR_OPENING:
      if (millis() - stateChangeTimestamp >= DOOR_MOVE_DURATION) {
        changeState(DOOR_OPEN);
        // No need to turn off relay, it was just a pulse
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
          triggerRelay(RELAY_CLOSE_PIN); // Send a pulse to close
          closeTimerTimestamp = 0;
        }
      }
      break;

    case DOOR_CLOSING:
      if (objectDetected) {
        changeState(DOOR_OPENING);
        triggerRelay(RELAY_OPEN_PIN); // Safety interrupt: send a pulse to open
      } else {
        if (millis() - stateChangeTimestamp >= DOOR_MOVE_DURATION) {
          changeState(DOOR_CLOSED);
          // No need to turn off relay, it was just a pulse
        }
      }
      break;
  }
}

// --- Helper Functions ---

bool isObjectDetected() {
  return (digitalRead(PHOTOCELL_PIN_1) == OBJECT_DETECTED || digitalRead(PHOTOCELL_PIN_2) == OBJECT_DETECTED);
}

// Triggers a non-blocking pulse on a relay pin
void triggerRelay(int pin) {
  if (pin == RELAY_OPEN_PIN && !openRelayPulseActive) {
    digitalWrite(RELAY_OPEN_PIN, HIGH);
    openRelayPulseActive = true;
    openRelayPulseStart = millis();
  } else if (pin == RELAY_CLOSE_PIN && !closeRelayPulseActive) {
    digitalWrite(RELAY_CLOSE_PIN, HIGH);
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
