/*
  Solution For:
  Topic: Can I have your opinion please!
  Category: Project Guidance
  Link: https://forum.arduino.cc/t/can-i-have-your-opinion-please/1123867

  Date: 22-May-23
  Author: MicroBeaut (μB)
*/

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define NUMBER_OF_POSITIONS 12  // NUMBER OF POSITIONS
#define DEVIDER 13.0f           // NUMBER OF RESISTORS
#define VOLTAGE 5.0f            // VOLTAGE MAL
#define RAW_MAX 1024.0f         // ANALOG RESOLUTION
#define ERROR_LIMIT 0.25f       // ERROR VOLTAGE LIMIT

#define analogInputPin A0
#define intervalTime 100UL

const float openCircuit = VOLTAGE / DEVIDER * 0.25;
const float shortCircuit = VOLTAGE - openCircuit;

LiquidCrystal_I2C lcd(0x27, 20, 2);

enum State {
  NORMAL,
  OPEN_CIRCUIT,
  SHORT_CIRCUTI
};

unsigned long startTime;
float analogValue;
uint8_t position;

State state;

void analogCondition();
State getState(float value);
uint8_t getPosition();
bool isInRange(float voltage, float reference);
bool isIntervalTimout();
void lcdDisplay();

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  analogCondition();
  position = getPosition();
  lcdDisplay();
}

void analogCondition() {
  int rawValue = analogRead(analogInputPin);
  analogValue = rawValue * VOLTAGE / RAW_MAX;
  state = getState(analogValue);
}

State getState(float value) {
  if (value <= openCircuit) return OPEN_CIRCUIT;
  if (value >= shortCircuit) return SHORT_CIRCUTI;
  return NORMAL;
}

uint8_t getPosition() {
  for (uint8_t index = 1; index <= NUMBER_OF_POSITIONS; index++) {
    float reference = index * VOLTAGE / DEVIDER;
    if (isInRange(analogValue, reference)) return index;
  }
  return 0;
}

bool isInRange(float voltage, float reference) {
  if (voltage > reference + ERROR_LIMIT) return false;
  if (voltage < reference - ERROR_LIMIT) return false;
  return true;
}

bool isIntervalTimout() {
  unsigned long elapsedTime = micros() - startTime;
  if (elapsedTime < intervalTime) return false;
  startTime = micros();
  return true;
}

void lcdDisplay() {
  if (!isIntervalTimout()) return;
  lcd.setCursor(0, 0);
  lcd.print("Voltage: ");
  lcd.print(String(analogValue, 3));
  lcd.setCursor(0, 1);
  lcd.print("Position: ");
  switch (state) {
    case OPEN_CIRCUIT:
      lcd.print("OC");
      break;
    case SHORT_CIRCUTI:
      lcd.print("SC");
      break;
    default:
      char pos[3];
      sprintf(pos, "%02d", position);
      lcd.print(pos);
      break;
  }
}