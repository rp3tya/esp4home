#include "esphome.h"

const uint8_t ANALOG_PIN = A0;
volatile bool halldiff = 0;

/**
 * Sensor.
 * Requires globals in YAML:
 *
 * - id: baseline
 *   type: int
 *   restore_value: yes
 *   initial_value: '542'
 * - id: threshold
 *   type: int
 *   restore_value: yes
 *   initial_value: '28'
 */
class AH3503BinarySensor : public PollingComponent, public BinarySensor {
 public:
  AH3503BinarySensor() : PollingComponent(1000) {}
  void setup() override {
    pinMode(ANALOG_PIN, INPUT);
  }
  void update() override {
    int b = baseline->value();
    int t = threshold->value();
    int a = analogRead(ANALOG_PIN);
    int d = abs(a - b);
    bool s = (d < t);
    //ESP_LOGI("main", "B:%d, A0:%d, D:%d, T:%d => %d", b, a, d, t, s);
    publish_state(s);
  }
};


