#include <Arduino.h>

#include "STM32Ethernet.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LiquidCrystal_I2C.h"

#include "Log.h"
#include "Led.h"
#include "Buzzer.h"
#include "Order.h"
#include "Payment.h"
#include "Reader.h"

#define SERIAL_BAUDRATE          (115200)

#define BUZZER_PIN               (0)
#define BUZZER_BEEP_DURATION_MS  (1000)
#define BUZZER_TONE_FREQUENCY_HZ (1000)

#define LCD_I2C_ADDRESS          (0x27)
#define LCD_CHARS_PER_LINE       (16)
#define LCD_LINES                (2)

static void cardReaderTask(void *parameters);
static void communicationTask(void *parameters);
static void vTask3(void *parameters);

static const IPAddress staticIP(192, 168, 1, 55);

static Buzzer buzzer(BUZZER_PIN);
static Log logger(&Serial);
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_CHARS_PER_LINE, LCD_LINES);

const char *TAG = "MAIN";

void setup() {
  logger.setup(SERIAL_BAUDRATE);

  xTaskCreate(cardReaderTask, "Card Reader Task", 6*1024, NULL, 1, NULL);
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

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("Welcome !");

  // Setup ethernet with DHCP else use a static IP
  vTaskDelay(pdMS_TO_TICKS(1000));
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

  // Re-initialize LCD
  delay(1000);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("Connected");

  // Register reader callback and start listening
  reader.onDetect([](byte *uid, byte length) {
    logger.i(TAG, "Detected card/phone/tag");
    lcd.clear();
    lcd.print("Detected card");

    Serial.print("UID:");
    for (byte i = 0; i < length; i++) {
      Serial.print(uid[i] < 0x10 ? " 0" : " ");
      Serial.print(uid[i], HEX);
    }
    Serial.println();
    buzzer.beep(250);
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

  // Payment payment;
  // Order order("f996ac47-daac-4a43-a9a6-4bc391edb317", 2, 3, 1693175157);

  // logger.d(TAG, "Initiating payment...");
  // payment.initiate(order, [](Payment::Error result) {
  //   if (result == Payment::Error::Success) {
  //     logger.i(TAG, "Payment successful!");
  //   } else {
  //     logger.e(TAG, "Payment failed (result=%d)", result);
  //   }
  // });

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
