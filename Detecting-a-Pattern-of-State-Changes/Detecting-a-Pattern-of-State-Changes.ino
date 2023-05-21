/*
  Solution For:
  Topic: Detecting a Pattern of State Changes
  Category: Programming Questions
  Link: https://forum.arduino.cc/t/detecting-a-pattern-of-state-changes/1126739/18

  Sketch: Detecting-a-Pattern-of-State-Changes.ino
  Date: 15-May-23
  Author: MicroBeaut (μB)
*/

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Analog Input
#define Pot1_Input_Pin A0
#define Pot2_Input_Pin A1
#define C_Detector_LDR_Pin A2
#define Motor_Voltage_Pin A3

// LED Output
#define Pot1_LED_Pin 8
#define Pot2_LED_Pin 7
#define C_Detector_LED_Pin 4
#define Motor_LED_Pin 3

// Servo Output (PWM)
#define Servo_Tensioner_Pin 11
#define Servo_Arm_Lock_Pin 10
#define Servo_Door_Pin 9
#define Servo_Dispenser_Pin 6

#define VOLTAGE_MAX 5.0f
#define PERCENT_ERROR 5.0f

#define PATTERN_TIMEOUT 10000UL
#define INTERVAL_TIME 100UL
#define BLINK_TIME 500UL
#define DECIMAL_POINT 1

// LDR Characteristics
#define GAMMA 0.7f
#define RL10  50.0f

enum State {
  SELECTION,
  PREDICATE,
  COMPLETED
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
} PatternState;

const uint8_t ledStatePins[] = {Pot1_LED_Pin, Pot2_LED_Pin, C_Detector_LED_Pin, C_Detector_LDR_Pin, Motor_LED_Pin};
uint8_t numberOfLEDs = sizeof(ledStatePins) / sizeof(uint8_t);

const uint8_t potPins[] = {Pot1_Input_Pin, Pot2_Input_Pin};
uint8_t numberOfPotPins = sizeof(potPins) / sizeof(uint8_t);

void Selection();
void PatternValidation();
void PatternCompleted();

typedef void (*StateCallback)(void);
StateCallback stateCallback[] = {
  Selection,
  PatternValidation,
  PatternCompleted
};

const float referenceA = 2.5f;
float voltagePatternA[] = {0.0f, referenceA, 0.0f, referenceA, 0.0f}; // Pattern A
const uint8_t sizePatternA = sizeof(voltagePatternA) / sizeof(float);

const float referenceB = 4.0f;
float voltagePatternB[] = {0.0f, referenceB, 0.0f, referenceB, 0.0f}; // Pattern B
const uint8_t sizePatternB = sizeof(voltagePatternB) / sizeof(float);

PatternState patterns[] = {
  {PREDICATE, COMPLETED, referenceA, voltagePatternA, sizePatternA},
  {PREDICATE, COMPLETED, referenceB, voltagePatternB, sizePatternB},
};
const uint8_t numberOfPatterns = sizeof(patterns) / sizeof(PatternState);

State state;
TimerState patternTimer;
TimerState intervalTimer;
TimerState blinkTimer;

Servo Tensioner;
Servo Arm_Lock;
Servo Door;
Servo Dispenser;

bool toggle;
float Pot1_Voltage;
float Pot2_Voltage;
float LDR_Voltage;
float Motor_Voltage;

uint8_t Counter;
int Pot_Chooser = -1;
bool Launch_Activated = false;

LiquidCrystal_I2C lcd(0x27, 20, 4);

void ServoInit(int tensionerAngle, int armlockAngel, int doorAngle, int dispenserAngle);
float AnalogScale(uint8_t pin);
bool PatternPredicate(float voltage, uint8_t id);
void SelectionTimeout();
bool AnalogCompare(float voltage, float reference, bool start = false);
void StartTimer(TimerState *timerState);
void StopTimer(TimerState *timerState);
bool Timer(TimerState *timerState);

void Launch();
void Blink();
void StatusLCD();
void StatusLED(uint8_t ledIndex);
String PadLeft(String message, int length, char paddingChar);

void setup() {
  for (uint8_t index = 0; index < numberOfLEDs; index++) {
    pinMode(ledStatePins[index], OUTPUT);
  }
  pinMode(LED_BUILTIN, OUTPUT);

  Tensioner.attach(Servo_Tensioner_Pin);
  Arm_Lock.attach(Servo_Arm_Lock_Pin);
  Door.attach(Servo_Door_Pin);
  Dispenser.attach(Servo_Dispenser_Pin);

  patternTimer.delayTime = PATTERN_TIMEOUT;
  intervalTimer.delayTime = INTERVAL_TIME;
  intervalTimer.autoReset = true;
  StartTimer(&intervalTimer);
  blinkTimer.delayTime = BLINK_TIME;
  blinkTimer.autoReset = true;
  StartTimer(&blinkTimer);

  lcd.init();
  lcd.backlight();

  state = SELECTION;            // Begin State = SELECTION
}

void loop() {
  Pot1_Voltage = AnalogScale(Pot1_Input_Pin);
  Pot2_Voltage = AnalogScale(Pot2_Input_Pin);
  LDR_Voltage = AnalogScale(C_Detector_LDR_Pin);
  Motor_Voltage = AnalogScale(Motor_Voltage_Pin);

  stateCallback[state]();
  Launch();
  Blink();
  StatusLCD();
}

void Launch() {
  // TODO: SOMETHING
  digitalWrite(Motor_LED_Pin, Launch_Activated);
  if (Launch_Activated) {
    ServoAngle(180, 0, 0, 0);
  } else {
    ServoAngle(0, 180, 180, 180); // Initial Servo Angle
  }
}

void Blink() {
  if (!Timer(&blinkTimer)) return;
  toggle = !toggle;
  digitalWrite(LED_BUILTIN, toggle);
}

void Selection() {
  StatusLED(0, false);
  for (uint8_t index = 0; index < numberOfPatterns; index++) {
    if (AnalogCompare(Motor_Voltage, patterns[index].reference, true)) {
      Pot_Chooser = index;
      state = patterns[Pot_Chooser].state;
      Launch_Activated = false;
      StatusLED(Pot_Chooser, true);
      Counter = 0;
    }
  }
}

void PatternValidation() {
  SelectionTimeout();
  float voltage = AnalogScale(potPins[Pot_Chooser]);
  bool value = PatternPredicate(voltage, Pot_Chooser);
  if (!value) return;
  StopTimer(&patternTimer);
  StatusLED(Pot_Chooser, false);
  state = patterns[Pot_Chooser].next;
}

void PatternCompleted() {
  // TODO: SOMETHING
  Launch_Activated = true;
  state = SELECTION;
}

bool PatternPredicate(float voltage, uint8_t id) {
  if (AnalogCompare(voltage, patterns[id].values[Counter])) {
    Counter++;
  }
  return Counter == patterns[id].size;
}

void SelectionTimeout() {
  if (Timer(&patternTimer)) {
    state = SELECTION;
  }
}

void ServoAngle(int tensionerAngle, int armlockAngel, int doorAngle, int dispenserAngle) {
  Tensioner.write(tensionerAngle);
  Arm_Lock.write(armlockAngel);
  Door.write(doorAngle);
  Dispenser.write(dispenserAngle);
}

float AnalogScale(uint8_t pin) {
  long rawValue = analogRead(pin);
  return rawValue * VOLTAGE_MAX / 1023;
}

bool AnalogCompare(float voltage, float reference, bool start) {
  float pctError = (voltage - reference ) * 100.0f / reference; // Error Percentage Calculation
  if (pctError > 0 && pctError > PERCENT_ERROR) return false;   // Compare an error with a positive error percentage
  if (pctError < 0 && -pctError > PERCENT_ERROR) return false;  // Compare an error with a negative error percentage
  if (start) StartTimer(&patternTimer);                         // Start Timout Timer
  return true;                                                  // The voltage value is in range.
}

float Lux(float voltage) {
  float resistance = 2000 * voltage / (1 - voltage / 5);
  return pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
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

void StatusLCD() {
  if (!Timer(&intervalTimer)) return;
  lcd.setCursor(0, 0);
  lcd.print("P1=");
  lcd.print(String(Pot1_Voltage, DECIMAL_POINT));
  lcd.setCursor(8, 0);
  lcd.print("P2=");
  lcd.print(String(Pot2_Voltage, DECIMAL_POINT));
  lcd.setCursor(0, 1);
  lcd.print("MT=");
  lcd.print(String(Motor_Voltage, DECIMAL_POINT));
  lcd.setCursor(8, 1);
  lcd.print("LD=");
  lcd.print(String(LDR_Voltage, DECIMAL_POINT));
  lcd.setCursor(0, 2);
  lcd.print("Lux:=");
  String lux = PadLeft(String(Lux(LDR_Voltage), DECIMAL_POINT), 9, ' ');
  lcd.print(lux);
  lcd.setCursor(0, 3);
  lcd.print("Ref:=");
  float reference = Pot_Chooser < 0 ? 0.0 : patterns[Pot_Chooser].values[Counter];
  lcd.print(String(reference, DECIMAL_POINT));
  lcd.setCursor(10, 3);
  lcd.print("C:=");
  lcd.print(Counter);
  lcd.setCursor(15, 3);
  lcd.print("A:=");
  lcd.print(Launch_Activated);

}

void StatusLED(uint8_t ledIndex, bool set) {
  if (ledIndex >= numberOfPatterns) return;
  for (uint8_t index = 0; index < numberOfPatterns; index++) {
    digitalWrite(ledStatePins[index], LOW);
  }
  if (set) digitalWrite(ledStatePins[ledIndex], HIGH);
}

// return string by padding charecter on the Left of the String
String PadLeft(String message, int length, char paddingChar) {
  int lenPad = length - message.length();
  for (int index = 0; index < lenPad; index++) {
    message = paddingChar + message;
  }
  return message.substring(0, length);
}