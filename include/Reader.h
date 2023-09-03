#ifndef READER_H
#define READER_H

#include <Arduino.h>
#include <functional>
#include <MFRC522.h>

class Reader {

public:
  Reader();
  void begin();
  void loop();
  void onDetect(std::function<void(byte *, byte)> callback);

private:
  MFRC522 _mfrc522;
  std::function<void(byte *, byte)> _detectCallback;
};

#endif // READER_H
