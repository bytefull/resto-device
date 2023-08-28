#ifndef BUZZER_H
#define BUZZER_H

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

#endif // BUZZER_H