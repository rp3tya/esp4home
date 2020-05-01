#include "esphome.h"

const uint8_t RX_PIN = 13;
volatile uint64_t pack = 0;

static inline unsigned int diff(int A, int B) {
  return abs(A - B);
}
ICACHE_RAM_ATTR void handler() {
  static uint64_t code = 0;
  static uint8_t cnt = 0;
  static unsigned long lastTime = 0;
  //
  const unsigned long time = micros();
  const unsigned long duration = time - lastTime;
  //
  if (duration > 400 && duration < 700) {
    if (digitalRead(RX_PIN) == LOW) {
      // pulse
    }
  } else if (duration > 700 && duration < 1000) {
    if (digitalRead(RX_PIN) == HIGH) {
      code <<= 1;
      cnt++;
      code |= 0;
    }
  } else if (duration > 1500 && duration < 2000) {
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
      uint32_t id = pack >> 28 & 0xFF;
      uint32_t fl = pack >> 24 & 0xF;
      uint32_t tm = pack >> 12 & 0xFFF;
      uint32_t cs = pack >> 8 & 0xF;
      uint32_t hm = pack >> 0 & 0xFF;
      pack = 0;
      //
      int fai = -1; // first available index
      int hci = -1; // house code index
      int hcv = -1; // house code value
      for (int i=0; i<sensor_count; i++) {
        hcv = house_code->value()[i];
        if (hcv == 0) {
          fai = (fai < 0) ? i : fai; // remember available index
        } else if (hcv == id) {
          hci = i; // remember device index
        }
      }
      if (hci == -1 && fai != -1) {
        // new house code, save to first available index
        hci = fai;
        house_code->value()[hci] = id;
      }
      if (hci >= 0 && hci < sensor_count) {
        // known house code, publish readings
        temperature_sensors[hci]->publish_state(tm/10.0);
        humidity_sensors[hci]->publish_state(hm);
      }
      //
      ESP_LOGI("main", "House Code: %d Temperature: %.1f Humidity: %d Index: %d", id, tm/10.0, hm, hci);
    }
  }
};

