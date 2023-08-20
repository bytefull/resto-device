#include "Arduino.h"
#include "Buzzer.h"

Buzzer::Buzzer(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void Buzzer::beep() {
  tone(_pin, 1000, 0xFFFFFFFF);
}

void Buzzer::beep(int duration) {
  tone(_pin, 1000, duration);
}

void Buzzer::stop() {
  noTone(_pin);
}
