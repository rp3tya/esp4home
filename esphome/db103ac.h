#include "esphome.h"

const uint8_t TX_PIN = D5;
const uint8_t RX_PIN = D6;
volatile unsigned long db103ac = 0;

/**
 * Transmit
 */
void transmit(int pin, String pac) {
  noInterrupts();
  pinMode(pin, OUTPUT);
  //
  digitalWrite(pin, HIGH);delayMicroseconds(1650);
  for (int r = 1; r <= 2; r++) {
    digitalWrite(pin, LOW); delayMicroseconds(1650);
    for (int i=0; i < pac.length(); i++) {
      if (pac[i] == '1') {
        digitalWrite(pin, HIGH);delayMicroseconds(100);
        digitalWrite(pin, LOW); delayMicroseconds(150);
      } else { // pac[i] == '0'
        digitalWrite(pin, HIGH);delayMicroseconds(200);
        digitalWrite(pin, LOW); delayMicroseconds(050);
      }
    }
  }
  //
  pinMode(pin, INPUT);
  interrupts();
}

/**
 * Receive
 */
static inline unsigned int diff(int A, int B) {
  return abs(A - B);
}
ICACHE_RAM_ATTR void handler() {
  //static uint8_t changeCount = 0;
  static unsigned long code = 0;
  static unsigned long lastTime = 0;
  //
  const unsigned long time = micros();
  const unsigned long duration = time - lastTime;

  if (duration > 150 && duration < 250) {
    if (digitalRead(RX_PIN) == HIGH) {
      code <<= 1;
      // one
      code  |= 1;
    }
  } else if (duration > 250 && duration < 650) {
    if (digitalRead(RX_PIN) == LOW) {
      code <<= 1;
      // zero
      if (code == 254) {
        db103ac = code;
      }
    }
  } else if (duration > 650) {
    code = 0;
  } else {
    code = 0;
  }
  //  
  lastTime = time;  
}

/**
 * Sensor
 */
class DB103ACBinarySensor : public Component, public BinarySensor {
 public:
  void setup() override {
    pinMode(RX_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(RX_PIN), handler, CHANGE);
  }
  void loop() override {
    publish_state(db103ac);
    if (db103ac > 0) {
      db103ac = 0;
    }
  }
};


