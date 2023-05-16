/*
  Solution For:
  Topic:      Multiple Multiplexers
  Category:   Programming Questions
  Link:       https://forum.arduino.cc/t/multiple-multiplexers/1124045/59

  Sketch:     Multiple-Multiplexers.ino
  Date:       13-May-23
  Author:     MicroBeaut (μB)
*/

#define select0Pin 2      // Select 0
#define select1Pin 3      // Select 1
#define select2Pin 4      // Select 2
#define select3Pin 5      // Select 3

#define commonInputAPin 6 // Common Input
#define commonInputBPin 7 // Common Input

#define bit0Pin 8   // Decoder Bit 0
#define bit1Pin 9   // Decoder Bit 1
#define bit2Pin 10  // Decoder Bit 2
#define bit3Pin 11  // Decoder Bit 3
#define bit4Pin 12  // Decoder Bit 4

#define NOT_USED_CHANNEL 0
const uint8_t selectPins[] = {select0Pin, select1Pin, select2Pin, select3Pin};  // Select Pin Array
const uint8_t commonInputPins[] = {commonInputAPin, commonInputBPin};           // Common Input Array
const uint8_t numberOfSelects = 4;                                              // Number Of Selection Pin
const uint8_t numberOfChannels = 16;                                            // Number Of Channels per Multiplexer/Demultiplexer
const uint8_t numberOfMultiplexers = 2;                                         // Number Of Multiplexer/Demultiplexer

// Declare Valiable for Example
const uint8_t decoderBits[] = {bit0Pin, bit1Pin, bit2Pin, bit3Pin, bit4Pin};    // Decoder Pin Array
const uint8_t numberOfDecoderBits = 5;                                          // Number of Decoder Bits

// Input State
typedef struct {
  bool input;               // Input State
  bool state;               // Output State
  unsigned long startTime;  // Debounce State Time
} SensorState;

const uint8_t numberOfSensors = numberOfMultiplexers * numberOfChannels;
SensorState inputs[numberOfSensors];      // Declare Debounce Input with State;
const unsigned long debounceTime = 10UL;  // Declare Debounce Time in milliseconds

// Declare Variable for Blink
#define blinkDelayTime 500UL
bool blink;               // Blink State
unsigned long startTime;  // Blink Start Time

void InputMultiplexer();                          // Declare Input Multiplexer Function
uint8_t SensorScan();                             // Declare Sensor Scan Function
void SelectChannel(uint8_t channel);              // Declare Select Channel Function
bool InputDebounce(bool input, uint8_t index);    // Declare Input Debounce Function
void Decoder(uint8_t value);                      // Declare Decoder Function
void Blink();                                     // Declare Blink Function

void setup() {
  // Serial.begin(115200);
  // Serial.println("4067 Analog Multiplexer/Demultiplexer");

  for (uint8_t index = 0; index < numberOfSelects; index++) {
    pinMode(selectPins[index], OUTPUT);               // Set select Pin Mode
  }

  for (uint8_t index = 0; index < numberOfMultiplexers; index++) {
    pinMode(commonInputPins[index], INPUT_PULLUP);    // Set Common Input Pin Mode
  }

  for (uint8_t index = 0; index < numberOfDecoderBits; index++) {
    pinMode(decoderBits[index], OUTPUT);              // Set Decoder Bit Pin Mode
  }

  pinMode(LED_BUILTIN, OUTPUT);   // Set LED_BUILTIN Pin Mode

  blink = false;                  // Initial Blink State
  startTime = millis();           // Set a start time
}

void loop() {
  InputMultiplexer(); // Call Input Multiplexer Function
  Blink();            // Call Blink Function
}

void InputMultiplexer() {
  uint8_t sensor = SensorScan();
  Decoder(sensor);
}

uint8_t SensorScan() {
  for (uint8_t index = 0; index < numberOfChannels; index++) {
    SelectChannel(index);                                         // Select Channel
    for (uint8_t multiplexer = 0; multiplexer < numberOfMultiplexers; multiplexer++) {
      uint8_t sensor = index + multiplexer * numberOfChannels;    // Calculate Sensor Number
      if (sensor == NOT_USED_CHANNEL) continue;
      bool state = !digitalRead(commonInputPins[multiplexer]);    // Read Input
      bool detected = InputDebounce(state, sensor);               // Debounce Input
      if (detected) return sensor;
    }
  }
  return 0;
}

void SelectChannel(uint8_t channel) {
  for (uint8_t channelIndex = 0; channelIndex < numberOfSelects; channelIndex++) {
    bool select = bitRead(channel, channelIndex);    // Read Bit State
    digitalWrite(selectPins[channelIndex], select);  // Write Select State
  }
}

bool InputDebounce(bool input, uint8_t index) {
  if (input != inputs[index].input) {                           // Compare if State Changed
    inputs[index].startTime = millis();                         // Set Start Time with current millis
  }
  inputs[index].input = input;                                  // Store current input to input state
  if (inputs[index].state == input) return inputs[index].state; // Return if the state does not change
  unsigned long elapsed = millis() - inputs[index].startTime;   // Calculate elapsed time
  if (elapsed < debounceTime) return inputs[index].state;       // Return if debounce does not complete
  inputs[index].state = input;                                  // Store current input to input state

  // if (input) Serial.println("Sensor " + String(index) + " Detected!");
  // if (!input) Serial.println  ("Sensor " + String(index) + " Undetected!");
  return inputs[index].state;
}

void Decoder(uint8_t value) {
  for (uint8_t index = 0; index < numberOfDecoderBits; index++) {
    digitalWrite(decoderBits[index], bitRead(value, index));    // Write Decoder State
  }
}

void Blink() {
  unsigned elapsedTime = millis() - startTime;
  if (elapsedTime < blinkDelayTime) return;
  startTime = millis();               // Set A New Start Time
  blink = !blink;                     // Togle Blink State
  digitalWrite(LED_BUILTIN, blink);   // Write Blink State to LED
}