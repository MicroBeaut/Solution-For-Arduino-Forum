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

LiquidCrystal_I2C lcd(0x27, 20, 2);

unsigned long startTime;
float analogValue;
uint8_t position;

void setup() {
  lcd.init();
  lcd.backlight();
}

void loop() {
  AnalogCondition();
  position = PositionDetection();
  Display();
}

void AnalogCondition() {
  int rawValue = analogRead(analogInputPin);
  analogValue = rawValue * VOLTAGE / RAW_MAX;
}

uint8_t PositionDetection() {
  for (uint8_t index = 1; index <= NUMBER_OF_POSITIONS; index++) {
    float reference = index * VOLTAGE / DEVIDER;
    if (AnalogCompare(analogValue, reference)) return index;
  }
  return 0;
}

void Display() {
  if (!Interval()) return;
  lcd.setCursor(0, 0);
  lcd.print("Voltage: ");
  lcd.print(String(analogValue, 3));
  lcd.setCursor(0, 1);
  lcd.print("Position: ");
  char pos[3];
  sprintf(pos, "%02d", position);
  lcd.print(pos);
}

bool Interval() {
  unsigned long elapsedTime = micros() - startTime;
  if (elapsedTime < intervalTime) return false;
  startTime = micros();
  return true;
}

bool AnalogCompare(float voltage, float reference) {
  if (voltage > reference + ERROR_LIMIT) return false;  // Compare an error with a positive error percentage
  if (voltage < reference - ERROR_LIMIT) return false;  // Compare an error with a negative error percentage
  return true;
}