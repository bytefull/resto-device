#include <Arduino.h>
#include <SPI.h>
#include "MFRC522.h"

#include "Reader.h"

#define MFRC522_RST_PIN          (5)
#define MFRC522_SPI_SS_PIN       (10)

Reader::Reader() {
}

void Reader::begin() {
  byte registerValue;

  // Initialize the SPI instance
  SPI.begin();

  // Initializes the MFRC522 chip
  _mfrc522 = MFRC522(MFRC522_SPI_SS_PIN, MFRC522_RST_PIN);
  _mfrc522.PCD_Init();
}

MFRC522::Uid Reader::getUid() {
  byte byteIndex;

  if (_mfrc522.PICC_IsNewCardPresent() && _mfrc522.PICC_ReadCardSerial()) {
    Serial.print("Card UID: ");
    for (byteIndex = 0; byteIndex < _mfrc522.uid.size; byteIndex++) {
      Serial.print(_mfrc522.uid.uidByte[byteIndex] < 0x10 ? " 0" : " ");
      Serial.print(_mfrc522.uid.uidByte[byteIndex], HEX);
    }
    Serial.println();
    _mfrc522.PICC_HaltA();
  }
}

void Reader::onCardDetected(std::function<void(Reader::Error)> callback) {
  _detectCallback = callback;
}
