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
  // Look for new cards
  if (!_mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!_mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Raise callback
  if (_detectCallback) {
    _detectCallback(_mfrc522.uid.uidByte, _mfrc522.uid.size);
  }
}

void Reader::onDetect(std::function<void(byte *, byte)> callback) {
  _detectCallback = callback;
}
