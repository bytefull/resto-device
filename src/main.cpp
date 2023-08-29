#include <Arduino.h>
#include <SPI.h>

#include "Log.h"
#include "Led.h"
#include "Buzzer.h"
#include "Order.h"
#include "Payment.h"

#include <STM32Ethernet.h>
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

static void cardReaderTask(void *parameters);
static void communicationTask(void *parameters);
static void vTask3(void *parameters);

static void onCardDetected(void);

static MFRC522 mfrc522(MFRC522_SPI_SS_PIN, MFRC522_RST_PIN);
static Buzzer buzzer(BUZZER_PIN);
static Log logger(&Serial);
static byte registerValue = 0x7F;
static volatile bool cardDetected = false;
static volatile uint32_t detectionCounter = 0;

const char *TAG = "MAIN";

void setup() {
  logger.setup(SERIAL_BAUDRATE);

  // xTaskCreate(cardReaderTask, "Card Reader Task", 1024, NULL, 1, NULL);
  xTaskCreate(communicationTask, "Communication", 5*1024, NULL, 1, NULL);
  // xTaskCreate(vTask3, "Task 3", 1024, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop() {
  delay(100);
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName) {
  while (true);
}

static void cardReaderTask(void *parameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(100);
  Led redLED(LED_RED);
  byte byteIndex;

  SPI.begin();
  mfrc522.PCD_Init();

  // Read and printout the MFRC522 version (valid values are 0x91 and 0x92)
  logger.i(TAG, "MFRC522 version: 0x%X", mfrc522.PCD_ReadRegister(mfrc522.VersionReg));

  pinMode(MFRC522_IRQ_PIN, INPUT_PULLUP);

  // Enable MFRC522 interrupt
  registerValue = 0xA0;
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, registerValue);

  attachInterrupt(digitalPinToInterrupt(MFRC522_IRQ_PIN), onCardDetected, FALLING);

  logger.i(TAG, "Started Card Reader Task");
  for(;;) {
    if (cardDetected) {
      // Once card is detected read its UID
      logger.i(TAG, "Card detected");
      logger.d(TAG, "detectionCounter: %lu\r\n", detectionCounter);
      mfrc522.PICC_ReadCardSerial();
      Serial.print("Card UID: ");
      for (byteIndex = 0; byteIndex < mfrc522.uid.size; byteIndex++) {
        Serial.print(mfrc522.uid.uidByte[byteIndex] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[byteIndex], HEX);
      }
      Serial.println();

      // Clear pending interrupt
      mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
      mfrc522.PICC_HaltA();
      cardDetected = false;

      // Make a sound
      buzzer.beep(BUZZER_BEEP_DURATION_MS);
    }

    // Activate reception
    mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
    mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
    mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);

    redLED.toggle();
    vTaskDelay(xDelay);
  }
}

static void communicationTask(void *parameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(500);
  Led greenLED(LED_GREEN);

  // Static IP address to use in case the DHCP client fails to get an address
  const IPAddress staticIP(192, 168, 1, 55);

  // Setup ethernet with DHCP else use a static IP
  if(Ethernet.begin() == 0) {
    logger.e(TAG, "Failed to configure Ethernet using DHCP");
    Serial.print("Configuring Ethernet using static IP: ");
    Serial.println(staticIP);
    Ethernet.begin(staticIP);
  } else {
    logger.i(TAG, "Ethernet is configured successfully using DHCP");
  }
  Serial.print("Connected to network. IP = ");
  Serial.println(Ethernet.localIP());


  Payment payment;
  Order order("56780afa-e02a-4f89-9a67-c70988ebd023", 2, 31, 1693175157);

  payment.initiate(order);

  payment.onResult([](Payment::Error result) {
    if (result == Payment::Error::Success) {
      logger.i(TAG, "Payment successful!");
    } else {
      logger.e(TAG, "Payment failed (result=%d)", result);
    }
  });

  for(;;) {
    greenLED.toggle();
    vTaskDelay(xDelay);
  }
}

static void vTask3(void *parameters) {
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
