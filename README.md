# resto-device
Embedded project for resto card reader

### Wiring

```mermaid
sequenceDiagram
BUZZER ->> NUCLEO: GND...............GND
BUZZER ->> NUCLEO: I/O...............D0
BUZZER ->> NUCLEO: VCC...............5v
NUCLEO ->> MFRC522: D13...............SCK
NUCLEO ->> MFRC522: D11...............MOSI
NUCLEO ->> MFRC522: D12...............MISO
NUCLEO ->> MFRC522: D1................IRQ
NUCLEO ->> MFRC522: GND...............GND
NUCLEO ->> MFRC522: D5................RST
NUCLEO ->> MFRC522: 3v3...............3.3v
```

### Final application vision
``` C
// main.cpp

#include <Arduino.h>

#include "App.h"

App app();

void setup() {
  app.run()
}

void loop() {}
```