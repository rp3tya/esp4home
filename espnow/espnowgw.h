#include "esphome.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

uint8_t macaddr[] = {0x00,0x00,0x00,0x00,0x00,0x00};
void initVariant() {
    WiFi.mode(WIFI_STA);
    esp_wifi_set_mac(ESP_IF_WIFI_STA, &macaddr[0]);
}

typedef struct struct_message {
  float temp;
  float hum;
  float pres;
} struct_message;

struct_message sensorData;

/**
 * Sensor
 */
float pack;
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
    esp_now_init();
    esp_now_register_recv_cb([](const uint8_t * mac, const uint8_t *incomingData, int len) {
      memcpy(&sensorData, incomingData, sizeof(sensorData));
      pack = sensorData.temp;
    });
  }

  void loop() override {
    if (pack > 0) {
      ESP_LOGI("main", "Message: %f", pack);
      temperature_sensors[0]->publish_state(pack);
      //
      pack = 0;
    }
  }
};

