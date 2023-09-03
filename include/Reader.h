#ifndef READER_H
#define READER_H

#include <Arduino.h>
#include <functional>
#include <MFRC522.h>

class Reader {

public:
  enum class Error {
    Success,
  };

  Reader();
  void begin();
  MFRC522::Uid getUid();
  void onCardDetected(std::function<void(Error)> callback);

private:
  MFRC522 _mfrc522;

  std::function<void(Error)> _detectCallback;
};

#endif // READER_H
