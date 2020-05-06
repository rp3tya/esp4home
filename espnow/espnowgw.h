#include "esphome.h"
#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  float temp;
  float hum;
  float pres;
} struct_message;

struct_message sensorData;
volatile uint64_t pack = 0;

/**
 * Sensor
 */
class EspNowSensor : public Component {
 private:
  int sensor_count;
  std::vector<Sensor*> temperature_sensors{};
 public:
  std::vector<Sensor*> sensors{};

  EspNowSensor() : Component() {
    sensor_count = sizeof(enid->value()) / sizeof(int);
    //
    for (int i=0; i<sensor_count; i++) {
      Sensor* sensor = new Sensor();
      temperature_sensors.push_back(sensor);
      sensors.push_back(sensor);
    }
  }

  void setup() override {
    WiFi.mode(WIFI_STA);

    esp_now_init();
    esp_now_register_recv_cb([](const uint8_t * mac, const uint8_t *incomingData, int len) {
      memcpy(&sensorData, incomingData, sizeof(sensorData));
      pack = sensorData.temp;
    });
  }

  void loop() override {
    if (pack > 0) {
      uint32_t id = pack;
      pack = 0;
      //
      temperature_sensors[0]->publish_state(id);
      //
      ESP_LOGI("main", "Message: %d", id);
    }
  }
};


