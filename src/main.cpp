#include <Arduino.h>
#include <SPI.h>

#include "Log.h"
#include "Led.h"
#include "Buzzer.h"
#include "MFRC522.h"

#include <STM32Ethernet.h>
#include <SSLClient.h>
#include <ArduinoJson.h>

#include "trust_anchors.h"

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

  // Choose the analog pin to get semi-random data from for SSL
  // Pick a pin that's not connected or attached to a randomish voltage source
  const int randomPin = A5;

  // Static IP address to use in case the DHCP client fails to get an address
  const IPAddress staticIP(192, 168, 1, 55);

  // Server URL
  const char *server = "restoken.azurewebsites.net";

  // Server port
  const uint16_t port = 443;

  // Bool used to track the LED state
  bool ledIsOn;

  // TCP client object to be used by the SSL client
  EthernetClient ethernetClient;

  // SSL client object
  SSLClient sslClient(ethernetClient, TAs, (size_t)TAs_NUM, randomPin, SSLClient::SSL_INFO);

  // Static JSON document object to parse the received message over HTTP
  StaticJsonDocument<512> requestJsonDoc;
  StaticJsonDocument<512> responseJsonDoc;

  char payload[512] = {0};

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

  // specify the server and port, 443 is the standard port for HTTPS
  logger.i(TAG, "Connecting to server...");
  if (sslClient.connect(server, port)) {
    Serial.print("Connected to ");
    Serial.println(ethernetClient.remoteIP());

    requestJsonDoc["customer_id"] = "56780afa-e02a-4f89-9a67-c70988ebd023";
    requestJsonDoc["restaurant_id"] = 1;
    requestJsonDoc["amount"] = 19;
    requestJsonDoc["timestamp"] = 1693175157;

    serializeJson(requestJsonDoc, payload);

    // Send HTTP POST request
    sslClient.println("POST /api/v1/payment HTTP/1.1");
    sslClient.println("User-Agent: SSLClientOverEthernet");
    sslClient.print("Host: ");
    sslClient.println(server);
    sslClient.println(F("Connection: close"));
    sslClient.print("Content-Length: ");
    sslClient.println(strlen(payload));
    sslClient.println();
    sslClient.println(payload);
    if (sslClient.println() == 0) {
      logger.e(TAG, "Failed to send request");
      sslClient.stop();
      while (true);
    }

    // Check HTTP status
    char status[32] = {0};
    sslClient.readBytesUntil('\r', status, sizeof(status));
    // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
    if (strcmp(status + strlen("HTTP/1.0 "), "200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      sslClient.stop();
      while (true);
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!sslClient.find(endOfHeaders)) {
     logger.e(TAG, "Invalid response, didn't find the end of headers");
      sslClient.stop();
      while (true);
    }

    // Parse JSON object from response
    DeserializationError error = deserializeJson(responseJsonDoc, sslClient);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      sslClient.stop();
      while (true);
    }

    // Extract values from JSON document
    Serial.println(F("Response:"));

    Serial.print(F("status: "));
    Serial.println(responseJsonDoc["status"].as<const char*>());

    // Disconnect
    delay(2000);
    sslClient.stop();

  } else {
    // Get the SSL error if any
    switch (sslClient.getWriteError()) {
    case sslClient.SSL_CLIENT_CONNECT_FAIL:
      logger.e(TAG, "The underlying client failed to connect, probably not an issue with SSL");
      break;
    case sslClient.SSL_BR_CONNECT_FAIL:
      logger.e(TAG, "BearSSL failed to complete the SSL handshake, check logs for bear ssl error output");
      break;
    case sslClient.SSL_CLIENT_WRTIE_ERROR:
      logger.e(TAG, "The underlying client failed to write a payload, probably not an issue with SSL");
      break;
    case sslClient.SSL_BR_WRITE_ERROR:
      logger.e(TAG, "An internal error occurred with BearSSL, check logs for diagnosis");
      break;
    case sslClient.SSL_INTERNAL_ERROR:
      logger.e(TAG, "An internal error occurred with SSLClient, and you probably need to submit an issue on Github");
      break;
    case sslClient.SSL_OUT_OF_MEMORY:
      logger.e(TAG, "SSLClient detected that there was not enough memory (>8000 bytes) to continue");
      break;
    default:
      Serial.print("Unknown error: ");
      Serial.println(sslClient.getWriteError());
      break;
    }
  }

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
