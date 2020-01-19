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
 private:
  int sensor_count;
  std::vector<Sensor*> temperature_sensors{};
  std::vector<Sensor*> humidity_sensors{};
 public:
  std::vector<Sensor*> sensors{};

  HCKK04Sensor() : Component() {
    sensor_count = sizeof(house_code->value()) / sizeof(int);
    //
    for (int i=0; i<sensor_count; i++) {
      Sensor* sensor = new Sensor();
      temperature_sensors.push_back(sensor);
      sensors.push_back(sensor);
    }
    for (int i=0; i<sensor_count; i++) {
      Sensor* sensor = new Sensor();
      humidity_sensors.push_back(sensor);
      sensors.push_back(sensor);
    }
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
      //
      int hci = -1;
      for (int i=0; i<sensor_count; i++) {
        if (id == house_code->value()[i]) {
          hci = i;
          temperature_sensors[i]->publish_state(tm/10.0);
          humidity_sensors[i]->publish_state(hm);
        }
      }
      //
      ESP_LOGI("main", "House Code: %d Temperature: %.1f Humidity: %d Index: %d", id, tm/10.0, hm, hci);
    }
  }
};
