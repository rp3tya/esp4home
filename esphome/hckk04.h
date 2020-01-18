#include "esphome.h"

const uint8_t RX_PIN = D6;
volatile uint64 pack = 0;

static inline unsigned int diff(int A, int B) {
  return abs(A - B);
}
ICACHE_RAM_ATTR void handler() {
  static uint64 code = 0;
  static uint8 cnt = 0;
  static unsigned long lastTime = 0;
  //
  const unsigned long time = micros();
  const unsigned long duration = time - lastTime;
  //
  if (duration > 500 && duration < 700) {
    if (digitalRead(RX_PIN) == LOW) {
      // pulse
    }
  } else if (duration > 700 && duration < 900) {
    if (digitalRead(RX_PIN) == HIGH) {
      code <<= 1;
      cnt++;
      code |= 0;
    }
  } else if (duration > 1500 && duration < 1900) {
    if (digitalRead(RX_PIN) == HIGH) {
      code <<= 1;
      cnt++;
      code |= 1;
    }
  } else if (duration > 3500) {
    if (cnt == 36) pack = code;
    code = 0;
    cnt = 0;
  } else {
    code = 0;
    cnt = 0;
  }
  //
  lastTime = time;
}

/**
 * Sensor
 */
class HCKK04Sensor : public Component {
 public:
  int sid = 0;
  Sensor *temperature_sensor = new Sensor();
  Sensor *humidity_sensor = new Sensor();
  HCKK04Sensor() : Component() {
  }
  void setup() override {
    pinMode(RX_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(RX_PIN), handler, CHANGE);
  }
  void loop() override {
    if (pack > 0) {
      uint32 id = pack >> 28 & 0xFF;
      uint32 fl = pack >> 24 & 0xF;
      uint32 tm = pack >> 12 & 0xFFF;
      uint32 cs = pack >> 8 & 0xF;
      uint32 hm = pack >> 0 & 0xFF;
      pack = 0;

      if (id == house_code->value()) {
        temperature_sensor->publish_state(tm/10.0);
        humidity_sensor->publish_state(hm);
      }

      ESP_LOGI("main", "House Code: %d   Temperature: %.1f   Humidity: %d", id, tm/10.0, hm);
    }
  }
};
