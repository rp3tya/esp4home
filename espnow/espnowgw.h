#include "esphome.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

uint8_t remoteMac[6];
uint8_t localMac[6];

void initVariant() {
    memcpy(localMac, &mac, sizeof(mac));
    WiFi.mode(WIFI_STA);
    esp_wifi_set_mac(ESP_IF_WIFI_STA, &localMac[0]);
}

struct_status myStatus;
struct_command myCommand;
int pack;

/**
 * Sensor
 */
class EspNowSensor : public PollingComponent {
 private:
  int sensor_count;
  std::vector<Sensor*> temperature_sensors{};
 public:
  std::vector<Sensor*> sensors{};

  EspNowSensor(int sid) : PollingComponent(5000) {
    sensor_count = sizeof(enid->value()) / sizeof(int);
    //
    for (int i=0; i<sensor_count; i++) {
      Sensor* sensor = new Sensor();
      temperature_sensors.push_back(sensor);
      sensors.push_back(sensor);
    }
  }

  void setup() override {
    memcpy(remoteMac, &mac, sizeof(mac)); remoteMac[5] = 1;
    //
    WiFi.mode(WIFI_STA);
    //
    esp_now_init();
    //
    esp_now_peer_info_t peeri;
    peeri.ifidx = ESP_IF_WIFI_STA;
    peeri.channel = 0;
    peeri.encrypt = 0;
    memcpy(peeri.peer_addr, &remoteMac, sizeof(remoteMac));
    esp_now_add_peer(&peeri);
    //
    esp_now_register_send_cb([](const uint8_t *mac_addr, esp_now_send_status_t state) {
      //
    });
    esp_now_register_recv_cb([](const uint8_t *mac_addr, const uint8_t *data, int len) {
      memcpy(&myStatus, data, sizeof(myStatus));
      pack = myStatus.switches;
    });
  }

  void update() override {
    temperature_sensors[0]->publish_state(pack);
    //
    myCommand.state = millis();
    sendState();
  }

void sendState() {
  uint8_t bs[sizeof(myCommand)];
  memcpy(bs, &myCommand, sizeof(myCommand));
  esp_now_send(NULL, bs, sizeof(myCommand));
}

};


