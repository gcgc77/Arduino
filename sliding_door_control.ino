// --- Pin Definitions ---
const int PHOTOCELL_PIN_1 = 2; // Input from the first photocell
const int PHOTOCELL_PIN_2 = 3; // Input from the second photocell
const int RELAY_OPEN_PIN = 4;  // Output to the relay that opens the door
const int RELAY_CLOSE_PIN = 5; // Output to the relay that closes the door

// --- Sensor Logic Level ---
// The E18-D80NK sensor is an NPN normally open sensor, which means its output
// goes LOW when it detects an object.
const int OBJECT_DETECTED = LOW;

// --- Time Constants (in milliseconds) ---
const unsigned long DOOR_MOVE_DURATION = 10000; // 10 seconds for the door to fully open or close
const unsigned long DOOR_CLOSE_DELAY = 10000;   // 10 seconds to wait before closing the door

// --- Door State Machine ---
enum DoorState {
  DOOR_CLOSED,
  DOOR_OPENING,
  DOOR_OPEN,
  DOOR_CLOSING
};

// --- Global Variables ---
DoorState currentDoorState = DOOR_CLOSED;
unsigned long stateChangeTimestamp = 0; // To track time spent in a state
unsigned long closeTimerTimestamp = 0; // To track the delay before closing

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Set photocell pins as inputs with internal pull-up resistors.
  // This is a robust way to handle digital inputs and avoids floating pins.
  pinMode(PHOTOCELL_PIN_1, INPUT_PULLUP);
  pinMode(PHOTOCELL_PIN_2, INPUT_PULLUP);

  // Set relay pins as outputs
  pinMode(RELAY_OPEN_PIN, OUTPUT);
  pinMode(RELAY_CLOSE_PIN, OUTPUT);

  // Ensure both relays are off at startup.
  // We assume the relays are ACTIVE-HIGH (HIGH turns them on).
  digitalWrite(RELAY_OPEN_PIN, LOW);
  digitalWrite(RELAY_CLOSE_PIN, LOW);

  Serial.println("Sliding Door Controller Initialized.");
  Serial.println("Current State: DOOR_CLOSED");
}

void loop() {
  // Read the state of the photocells at the beginning of each loop
  bool objectDetected = isObjectDetected();

  // The main state machine logic
  switch (currentDoorState) {
    case DOOR_CLOSED:
      // If an object is detected, start opening the door
      if (objectDetected) {
        changeState(DOOR_OPENING);
        digitalWrite(RELAY_OPEN_PIN, HIGH); // Activate open relay
      }
      break;

    case DOOR_OPENING:
      // If the door has been opening for the required duration, it is now fully open
      if (millis() - stateChangeTimestamp >= DOOR_MOVE_DURATION) {
        changeState(DOOR_OPEN);
        digitalWrite(RELAY_OPEN_PIN, LOW); // Deactivate open relay
      }
      break;

    case DOOR_OPEN:
      // If an object is detected, keep the door open by resetting the close timer
      if (objectDetected) {
        closeTimerTimestamp = 0; // Reset the timer
      } else {
        // If no object is detected, start the countdown to close the door
        if (closeTimerTimestamp == 0) {
          closeTimerTimestamp = millis(); // Start the timer
        }
        // If the countdown is complete, start closing the door
        if (millis() - closeTimerTimestamp >= DOOR_CLOSE_DELAY) {
          changeState(DOOR_CLOSING);
          digitalWrite(RELAY_CLOSE_PIN, HIGH); // Activate close relay
          closeTimerTimestamp = 0; // Reset timer
        }
      }
      break;

    case DOOR_CLOSING:
      // SAFETY FEATURE: If an object is detected while closing, immediately reopen the door
      if (objectDetected) {
        changeState(DOOR_OPENING);
        digitalWrite(RELAY_CLOSE_PIN, LOW);  // Deactivate close relay
        digitalWrite(RELAY_OPEN_PIN, HIGH); // Activate open relay
      } else {
        // If the door has been closing for the required duration, it is now fully closed
        if (millis() - stateChangeTimestamp >= DOOR_MOVE_DURATION) {
          changeState(DOOR_CLOSED);
          digitalWrite(RELAY_CLOSE_PIN, LOW); // Deactivate close relay
        }
      }
      break;
  }
}

// --- Helper Functions ---

// Checks if either photocell detects an object
bool isObjectDetected() {
  return (digitalRead(PHOTOCELL_PIN_1) == OBJECT_DETECTED || digitalRead(PHOTOCELL_PIN_2) == OBJECT_DETECTED);
}

// Handles state transitions and prints debug information
void changeState(DoorState newState) {
  currentDoorState = newState;
  stateChangeTimestamp = millis();

  Serial.print("State changed to: ");
  switch (newState) {
    case DOOR_CLOSED:
      Serial.println("DOOR_CLOSED");
      break;
    case DOOR_OPENING:
      Serial.println("DOOR_OPENING");
      break;
    case DOOR_OPEN:
      Serial.println("DOOR_OPEN");
      break;
    case DOOR_CLOSING:
      Serial.println("DOOR_CLOSING");
      break;
  }
}
