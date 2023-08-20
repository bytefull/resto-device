#ifndef Buzzer_h
#define Buzzer_h

#include "Arduino.h"

class Buzzer {

public:
  Buzzer(int pin);
  void beep();
  void beep(int duration);
  void stop();

private:
  int _pin;

};

#endif // Buzzer_h