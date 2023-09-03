#include <Arduino.h>
#include <SPI.h>

#include "MFRC522.h"

#include "Reader.h"

#define MFRC522_RST_PIN    (5)
#define MFRC522_SPI_SS_PIN (10)

Reader::Reader() {
}

void Reader::begin() {
  // Initialize the SPI instance
  SPI.begin();

  // Initializes the MFRC522 chip
  _mfrc522 = MFRC522(MFRC522_SPI_SS_PIN, MFRC522_RST_PIN);
  _mfrc522.PCD_Init();
}

void Reader::loop() {
  if (_mfrc522.PICC_IsNewCardPresent() && _mfrc522.PICC_ReadCardSerial()) {
    // Raise callback
    if (_detectCallback) {
      _detectCallback(_mfrc522.uid.uidByte, _mfrc522.uid.size);
    }

    // Halt PICC
    _mfrc522.PICC_HaltA();

    // Stop encryption on PCD
    _mfrc522.PCD_StopCrypto1();
  }
}

void Reader::onDetect(std::function<void(byte *, byte)> callback) {
  _detectCallback = callback;
}
