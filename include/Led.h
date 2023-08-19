#ifndef Led_h
#define Led_h

#include "Arduino.h"

class Led {

public:
  Led(int pin);
  void blink(int blinkRate);
  void on();
  void off();
  void toggle();

private:
  int _pin;

};

#endif // Led_h