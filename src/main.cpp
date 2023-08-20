#include <Arduino.h>
#include <SPI.h>

#include "Log.h"
#include "Led.h"
#include "Buzzer.h"
#include "MFRC522.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "event_groups.h"

#define SERIAL_BAUDRATE          (115200)

#define MFRC522_RST_PIN          (5)
#define MFRC522_SPI_SS_PIN       (10)
#define MFRC522_IRQ_PIN          (1)

#define BUZZER_PIN               (0)
#define BUZZER_BEEP_DURATION_MS  (1000)
#define BUZZER_TONE_FREQUENCY_HZ (1000)

static void vTask1(void *pvParameters);
static void vTask2(void *pvParameters);
static void vTask3(void *pvParameters);

static void onCardDetected(void);

static MFRC522 mfrc522(MFRC522_SPI_SS_PIN, MFRC522_RST_PIN);
static Buzzer buzzer(BUZZER_PIN);
static Log logger(&Serial);
static byte registerValue = 0x7F;
static volatile bool cardDetected = false;
static volatile uint32_t detectionCounter = 0;

const char *TAG = "MAIN";

void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  xTaskCreate(vTask1, "Task 1", 1024, NULL, 1, NULL);
  xTaskCreate(vTask2, "Task 2", 1024, NULL, 1, NULL);
  xTaskCreate(vTask3, "Task 3", 1024, NULL, 1, NULL);

  SPI.begin();
  mfrc522.PCD_Init();

  // Read and printout the MFRC522 version (valid values are 0x91 and 0x92)
  logger.i(TAG, "Version: 0x%X", mfrc522.PCD_ReadRegister(mfrc522.VersionReg));

  pinMode(MFRC522_IRQ_PIN, INPUT_PULLUP);

  // Enable MFRC522 interrupt
  registerValue = 0xA0;
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, registerValue);

  attachInterrupt(digitalPinToInterrupt(MFRC522_IRQ_PIN), onCardDetected, FALLING);

  vTaskStartScheduler();
}

void loop() {
  byte byteIndex;

  if (cardDetected) {
    cardDetected = false;

    // Once card is detected read its UID
    logger.i(TAG, "Card detected");
    logger.d(TAG, "detectionCounter: %lu\r\n", detectionCounter);
    mfrc522.PICC_ReadCardSerial();
    Serial.print(F("Card UID: "));
    for (byteIndex = 0; byteIndex < mfrc522.uid.size; byteIndex++) {
      Serial.print(mfrc522.uid.uidByte[byteIndex] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[byteIndex], HEX);
    }
    Serial.println();

    // Clear pending interrupt
    mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
    mfrc522.PICC_HaltA();

    // Make a sound
    buzzer.beep(BUZZER_BEEP_DURATION_MS);
  }

  // Activate reception
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);

  delay(100);
}

void vApplicationIdleHook(void) {
  loop();
}

void vApplicationTickHook(void) {
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName) {
  for(;;);
}

static void vTask1(void *pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(500);
  Led redLED(LED_RED);

  logger.i(TAG, "Started Task 1");
  for(;;) {
    redLED.toggle();
    vTaskDelay(xDelay);
  }
}

static void vTask2(void *pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(500);
  Led greenLED(LED_GREEN);

  logger.i(TAG, "Started Task 2");
  for(;;) {
    greenLED.toggle();
    vTaskDelay(xDelay);
  }
}

static void vTask3(void *pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(1000);
  Led blueLED(LED_BLUE);

  logger.i(TAG, "Started Task 3");
  for(;;) {
    blueLED.toggle();
    vTaskDelay(xDelay);
  }
}

static void onCardDetected(void) {
  cardDetected = true;
  detectionCounter++;
}
