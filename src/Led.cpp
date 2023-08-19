#include "Arduino.h"
#include "Led.h"

Led::Led(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void Led::blink(int blinkRate) {
  digitalWrite(_pin, HIGH);
  delay(blinkRate);
  digitalWrite(_pin, LOW);
  delay(blinkRate);
}

void Led::on() {
  digitalWrite(_pin, HIGH);
}

void Led::off() {
  digitalWrite(_pin, LOW);
}

void Led::toggle() {
  digitalToggle(_pin);
}
