#ifndef LED_H
#define LED_H

#include "Arduino.h"

class Led {

public:
  Led(int pin);
  void on();
  void off();
  void toggle();

private:
  int _pin;

};

#endif // LED_H