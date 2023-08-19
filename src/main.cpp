// /**
//  * ----------------------------------------------------------------------------
//  * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
//  * for further details and other examples.
//  * 
//  * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
//  * 
//  * Released into the public domain.
//  * ----------------------------------------------------------------------------
//  * Minimal example how to use the interrupts to read the UID of a MIFARE Classic PICC
//  * (= card/tag).
//  * 
//  * 
//  * Typical pin layout used:
//  * -----------------------------------------------------------------------------------------
//  *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
//  *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
//  * Signal      Pin          Pin           Pin       Pin        Pin              Pin
//  * -----------------------------------------------------------------------------------------
//  * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
//  * SPI SS      SDA(SS)      10            53        D10        3                10
//  * IRQ         ?            ?             ?         ?          2                10
//  * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
//  * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
//  * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
//  *
//  * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
//  * 
//  */

// #include <SPI.h>
// #include <MFRC522.h>

// #define RST_PIN         5           // Configurable, see typical pin layout above
// #define SS_PIN          10          // Configurable, see typical pin layout above
// #define IRQ_PIN         1           // Configurable, depends on hardware

// MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// MFRC522::MIFARE_Key key;

// volatile bool bNewInt = false;
// byte regVal = 0x7F;
// void activateRec(MFRC522 mfrc522);
// void clearInt(MFRC522 mfrc522);
// void readCard();
// void dump_byte_array(byte *buffer, byte bufferSize);

// /**
//  * Initialize.
//  */
// void setup() {
//   Serial.begin(115200); // Initialize serial communications with the PC
//   while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
//   SPI.begin();          // Init SPI bus

//   mfrc522.PCD_Init(); // Init MFRC522 card

//   /* read and printout the MFRC522 version (valid values 0x91 & 0x92)*/
//   Serial.print(F("Ver: 0x"));
//   byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
//   Serial.println(readReg, HEX);

//   /* setup the IRQ pin*/
//   pinMode(IRQ_PIN, INPUT_PULLUP);

//   /*
//    * Allow the ... irq to be propagated to the IRQ pin
//    * For test purposes propagate the IdleIrq and loAlert
//    */
//   regVal = 0xA0; //rx irq
//   mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);

//   bNewInt = false; //interrupt flag

//   /*Activate the interrupt*/
//   attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);

//   // do { //clear a spourious interrupt at start
//   //   ;
//   // } while (!bNewInt);
//   // bNewInt = false;

//   Serial.println(F("End setup"));
// }

// /**
//  * Main loop.
//  */
// void loop() {
//   if (bNewInt) { //new read interrupt
//     Serial.print(F("Interrupt. "));
//     mfrc522.PICC_ReadCardSerial(); //read the tag data
//     // Show some details of the PICC (that is: the tag/card)
//     Serial.print(F("Card UID:"));
//     dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
//     Serial.println();

//     clearInt(mfrc522);
//     mfrc522.PICC_HaltA();
//     bNewInt = false;
//   }

//   // The receiving block needs regular retriggering (tell the tag it should transmit??)
//   // (mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg,mfrc522.PICC_CMD_REQA);)
//   activateRec(mfrc522);
//   delay(100);
// } //loop()

// /**
//  * Helper routine to dump a byte array as hex values to Serial.
//  */
// void dump_byte_array(byte *buffer, byte bufferSize) {
//   for (byte i = 0; i < bufferSize; i++) {
//     Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//     Serial.print(buffer[i], HEX);
//   }
// }
// /**
//  * MFRC522 interrupt serving routine
//  */
// void readCard() {
//   bNewInt = true;
// }

// /*
//  * The function sending to the MFRC522 the needed commands to activate the reception
//  */
// void activateRec(MFRC522 mfrc522) {
//   mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
//   mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
//   mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
// }

// /*
//  * The function to clear the pending interrupt bits after interrupt serving routine
//  */
// void clearInt(MFRC522 mfrc522) {
//   mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
// }

#include <Arduino.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "event_groups.h"

void vTask1(void *pvParameters);
void vTask2(void *pvParameters);
void vTask3(void *pvParameters);

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  Serial.begin(115200);
  Serial.println("Hello world!");

  xTaskCreate(vTask1, "Task 1", 1024, NULL, 1, NULL);
  xTaskCreate(vTask2, "Task 2", 1024, NULL, 1, NULL);
  xTaskCreate(vTask3, "Task 3", 1024, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop() {
}

void vTask1(void *pvParameters)
{
  const TickType_t xDelayInMs = pdMS_TO_TICKS(500);

  Serial.println("Started Task 1");
  for(;;)
  {
    Serial.println("Running Task 1...");
    digitalToggle(LED_RED);
    vTaskDelay(xDelayInMs);
  }
}

void vTask2(void *pvParameters)
{
  const TickType_t xDelayInMs = pdMS_TO_TICKS(500);

  Serial.println("Started Task 2");
  for(;;)
  {
    Serial.println("Running Task 2...");
    digitalToggle(LED_GREEN);
    vTaskDelay(xDelayInMs);
  }
}

void vTask3(void *pvParameters)
{
  const TickType_t xDelayInMs = pdMS_TO_TICKS(1000);

  Serial.println("Started Task 3");
  for(;;)
  {
    Serial.println("Running Task 3...");
    digitalToggle(LED_BLUE);
    vTaskDelay(xDelayInMs);
  }
}

void vApplicationIdleHook(void)
{
}

void vApplicationTickHook(void)
{
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
  for(;;);
}
