#include <espnow.h>

uint8_t remoteMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct struct_message {
  float temp;
  float hum;
  float pres;
} struct_message;

struct_message sensorData;

void setup() {
  Serial.begin(115200);
  Serial.println();
  //
  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    Serial.print("send done, status = ");
    Serial.println(sendStatus);
  });
}

void loop() {
  sensorData.temp = uint32_t(millis());
  //
  uint8_t bs[sizeof(sensorData)];
  memcpy(bs, &sensorData, sizeof(sensorData));
  esp_now_send(NULL, bs, sizeof(sensorData));
  //
  delay(5000);
}
