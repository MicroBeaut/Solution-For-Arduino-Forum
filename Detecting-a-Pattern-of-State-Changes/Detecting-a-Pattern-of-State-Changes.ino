/*
  Solution For:
  Topic:      Detecting a Pattern of State Changes
  Category:   Programming Questions
  Link:       https://forum.arduino.cc/t/detecting-a-pattern-of-state-changes/1126739/18

  Sketch:     Detecting-a-Pattern-of-State-Changes.ino
  Date:       15-May-23
  Author:     MicroBeaut (μB)
*/

#include <Arduino.h>

#define ledPatternAPin 8
#define ledPatternBPin 9
#define ledStateAPin 10
#define ledStateBPin 11
#define ledSelectionPin 12
#define analogInputPin A0

#define VOLTAGE_MAX 5.0F
#define PERCENT_ERROR 0.1F

#define SELECTION_TIMEOUT 5000UL
#define STATE_TIMEOUT 10000UL
#define BLINK_DELAY_TIME 500UL
#define DECIMAL_POINT 1

typedef void (*StateCallback)(void);

enum State {
  PATTERN_A,
  PATTERN_B,
  STATE_A,
  STATE_B,
  SELECTION
};

typedef struct {
  bool start;
  bool timeout;
  bool autoReset;
  unsigned long startTime;
  unsigned long delayTime;
} TimerState;

typedef struct {
  State state;
  State next;
  float reference;
  float *values;
  uint8_t size;
  uint8_t count;
} PatternState;

State state;
TimerState patternTimer;
TimerState intervalTimer;
TimerState stateTimer;

uint8_t ledStatePins[] = {ledPatternAPin, ledPatternBPin, ledStateAPin, ledStateBPin, ledSelectionPin};
uint8_t numberOfLEDs = sizeof(ledStatePins) / sizeof(uint8_t);

bool toggle;
float voltage;

void Pattern();
void StateA();
void StateB();
void Selection();

StateCallback stateCallback[] = {Pattern,
                                 Pattern,
                                 StateA,
                                 StateB,
                                 Selection
                                };

const float referenceA = 2.5f;
float voltagePatternA[] = {0.0f, referenceA, 0.0f, referenceA};  // Pattern A
const uint8_t sizePatternA = sizeof(voltagePatternA) / sizeof(float);

const float referenceB = 4.0f;
float voltagePatternB[] = {0.0f, referenceB, 0.0f, referenceB};  // Pattern B
const uint8_t sizePatternB = sizeof(voltagePatternB) / sizeof(float);

PatternState patterns[] = {
  {PATTERN_A, STATE_A, referenceA, voltagePatternA, sizePatternA},
  {PATTERN_B, STATE_B, referenceB, voltagePatternB, sizePatternB},
};
const uint8_t numberOfPatterns = sizeof(patterns) / sizeof(PatternState);

float AnalogRead();
bool PatternPredicate(uint8_t id);
void SelectionTimeout();
bool AnalogCompare(float voltage, float reference, bool start = false);
void StartTimer(TimerState *timerState);
void StopTimer(TimerState *timerState);
bool Timer(TimerState *timerState);
void LEDState(uint8_t ledIndex);

void setup() {
  Serial.begin(115200);
  Serial.println("Analog Pattern");
  for (uint8_t index = 0; index < numberOfLEDs; index++) {
    pinMode(ledStatePins[index], OUTPUT);
  }
  pinMode(LED_BUILTIN, OUTPUT);

  stateTimer.delayTime = STATE_TIMEOUT;
  patternTimer.delayTime = SELECTION_TIMEOUT;
  intervalTimer.delayTime = BLINK_DELAY_TIME;
  intervalTimer.autoReset = true;
  StartTimer(&intervalTimer);

  state = SELECTION;  // Begin State = SELECTION
}

void loop() {
  voltage = AnalogRead();
  stateCallback[state]();
  SelectionTimeout();

  if (Timer(&intervalTimer)) {
    toggle = !toggle;
    digitalWrite(LED_BUILTIN, toggle);
  }
  Timer(&stateTimer);
}

void Selection() {
  for (uint8_t index = 0; index < numberOfPatterns; index++) {
    if (AnalogCompare(voltage, patterns[index].reference, true)) {
      state = patterns[index].state;
      patterns[index].count = 0;

      Serial.println();
      Serial.println("Pattern " + String(patterns[state].reference, DECIMAL_POINT) + " Selected");
      Serial.print("Count: 0 ");
    }
  }
  LEDState(state);
}

void Pattern() {
  LEDState(state);
  bool value = PatternPredicate(state);
  if (!value) return;
  StopTimer(&patternTimer);

  Serial.println();
  Serial.println("Pattern " + String(patterns[state].reference, DECIMAL_POINT) + "V Complated");
  Serial.println("State " + String(patterns[state].reference, DECIMAL_POINT) + "V Running for " + String(STATE_TIMEOUT / 1000)  + " Seconds");

  state = patterns[state].next;
  StartTimer(&stateTimer);
}

void StateA() {
  // TODO: SOMETHING

  // EXAMPLE LED BLINKING FOR 10 SECONDS
  digitalWrite(ledStatePins[state], toggle);
  if (!stateTimer.timeout) return;            // State Timeout
  digitalWrite(ledStatePins[state], LOW);

  Serial.println("State " + String(patterns[state - 2].reference, DECIMAL_POINT) + "V End");

  // Re-Select
  state = SELECTION;
}
void StateB() {
  // TODO: SOMETHING

  // EXAMPLE LED BLINKING FOR 10 SECONDS
  digitalWrite(ledStatePins[state], toggle);
  if (!stateTimer.timeout) return;            // State Timeout
  digitalWrite(ledStatePins[state], LOW);

  Serial.println("State " + String(patterns[state - 2].reference, DECIMAL_POINT) + "V End");

  // Re-Select
  state = SELECTION;
}

bool PatternPredicate(uint8_t id) {
  if (AnalogCompare(voltage, patterns[id].values[patterns[id].count])) {
    patterns[id].count++;

    Serial.print(String(patterns[id].count) + " ");
  }
  return patterns[id].count == patterns[id].size;
}

void SelectionTimeout() {
  if (Timer(&patternTimer)) {
    state = SELECTION;

    Serial.println();
    Serial.println("Pattern Timeout!");
  }
}

float AnalogRead() {
  long rawValue = analogRead(analogInputPin);
  return rawValue * VOLTAGE_MAX / 1023;
}

bool AnalogCompare(float voltage, float reference, bool start) {
  float pctError = (voltage - reference ) * 100.0f / reference; // Error Percentage Calculation
  if (pctError > 0 && pctError > PERCENT_ERROR) return false;   // Compare an error with a positive error percentage
  if (pctError < 0 && -pctError > PERCENT_ERROR) return false;  // Compare an error with a negative error percentage
  if (start) StartTimer(&patternTimer);                         // Start Timout Timer
  return true;                                                  // The voltage value is in range.
}

void StartTimer(TimerState *timerState) {
  timerState->startTime = millis();
  timerState->start = true;
}

void StopTimer(TimerState *timerState) {
  timerState->start = false;
}

bool Timer(TimerState *timerState) {
  if (timerState->timeout) timerState->timeout = false;
  if (!timerState->start) return false;
  unsigned long elapsedTime = millis() - timerState->startTime;
  if (elapsedTime < timerState->delayTime) return false;
  timerState->startTime = millis();
  timerState->start = timerState->autoReset;
  timerState->timeout = true;
  return true;
}

void LEDState(uint8_t ledIndex) {
  for (uint8_t index = 0; index < numberOfLEDs; index++) {
    if (index == ledIndex) continue;
    digitalWrite(ledStatePins[index], LOW);
  }
  digitalWrite(ledStatePins[ledIndex], HIGH);
}