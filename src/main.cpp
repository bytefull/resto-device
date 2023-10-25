#include <Arduino.h>

#include "STM32Ethernet.h"
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

static const IPAddress staticIP(192, 168, 1, 55);

static volatile bool initiatePayment = false;

static Buzzer buzzer(BUZZER_PIN);
static Led redLED(LED_RED);
static Log logger(&Serial);
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_CHARS_PER_LINE, LCD_LINES);
Reader reader;

const char *TAG = "MAIN";

void setup() {
  logger.setup(SERIAL_BAUDRATE);

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("Welcome !");

  // Setup ethernet with DHCP else use a static IP
  delay(1000);
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

    initiatePayment = true;
  });
  reader.begin();
}

void loop() {
  if (initiatePayment) {
    Payment payment;
    Order order("f996ac47-daac-4a43-a9a6-4bc391edb317", 2, 3, 1693175157);

    logger.d(TAG, "Initiating payment...");
    lcd.clear();
    lcd.print("Payment...");
    payment.initiate(order, [](Payment::Error result) {
      lcd.clear();
      if (result == Payment::Error::Success) {
        logger.i(TAG, "Payment successful!");
        lcd.print("Payment OK");
      } else {
        logger.e(TAG, "Payment failed (result=%d)", result);
        lcd.print("Payment NOK");
      }
    });

    initiatePayment = false;
  }

  reader.loop();
  redLED.toggle();
  delay(100);
}
