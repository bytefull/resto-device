#include <Arduino.h>
#include <SPI.h>

#include "Log.h"
#include "Led.h"
#include "Buzzer.h"
#include "Order.h"
#include "Payment.h"
#include "Reader.h"

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

static void onDetect(void);

static MFRC522 mfrc522(MFRC522_SPI_SS_PIN, MFRC522_RST_PIN);
static Buzzer buzzer(BUZZER_PIN);
static Log logger(&Serial);
static byte registerValue = 0x7F;
static volatile bool cardDetected = false;
static volatile uint32_t detectionCounter = 0;

const char *TAG = "MAIN";

void setup() {
  logger.setup(SERIAL_BAUDRATE);

  xTaskCreate(cardReaderTask, "Card Reader Task", 1024, NULL, 1, NULL);
  // xTaskCreate(communicationTask, "Communication", 5*1024, NULL, 1, NULL);
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
  Reader reader;

  reader.onDetect([](byte *uid, byte length) {
    logger.i(TAG, "Detected card/phone/tag");
    Serial.print("UID: ");
    for (byte i = 0; i < length; i++) {
      Serial.print(uid[i] < 0x10 ? " 0" : " ");
      Serial.print(uid[i], HEX);
    }
    Serial.println();
  });

  reader.begin();

  for(;;) {
    reader.loop();
    redLED.toggle();
    vTaskDelay(xDelay);
  }
}

static void communicationTask(void *parameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(3000);
  Led greenLED(LED_GREEN);

  vTaskDelay(xDelay);

  // Static IP address to use in case the DHCP client fails to get an address
  const IPAddress staticIP(192, 168, 1, 55);

  // Setup ethernet with DHCP else use a static IP
  logger.d(TAG, "Getting an IP address from LAN...");
  if(Ethernet.begin() == 0) {
    logger.e(TAG, "Failed to configure Ethernet using DHCP");
    Serial.print("Configuring Ethernet using static IP: ");
    Serial.println(staticIP);
    Ethernet.begin(staticIP);
  } else {
    logger.i(TAG, "Ethernet is configured successfully using DHCP");
  }
  logger.i(TAG, "Got an IP address");
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());


  Payment payment;
  Order order("f996ac47-daac-4a43-a9a6-4bc391edb317", 2, 3, 1693175157);

  logger.d(TAG, "Initiating payment...");
  payment.initiate(order, [](Payment::Error result) {
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

static void onDetect(void) {
  cardDetected = true;
  detectionCounter++;
}
