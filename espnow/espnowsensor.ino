#include "espnow_common.h"
#include <espnow.h>
#include <ESP8266WiFi.h>

uint8_t remoteMac[6];
uint8_t localMac[6];

struct_status myStatus;
struct_command myCommand;

void setup() {
  Serial.begin(115200);
  Serial.println();
  //
  memcpy(remoteMac, &mac, sizeof(mac));
  memcpy(localMac, &mac, sizeof(mac));
  localMac[5] = 1;
  //
  WiFi.mode(WIFI_STA);
  wifi_set_macaddr(0, const_cast<uint8*>(localMac));  
  Serial.println(WiFi.macAddress());

  esp_now_init();
  //
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
  //
  esp_now_register_send_cb([](uint8_t* mac_addr, uint8_t state) {
    Serial.print("Send "); 
    Serial.println(state);
  });
  esp_now_register_recv_cb([](uint8_t *mac_addr, uint8_t *data, uint8_t len) {
    Serial.print("Received "); Serial.print(len);
    memcpy(&myCommand, data, sizeof(myCommand));
    Serial.print(" bytes: "); Serial.println(myCommand.state);
  });
  
}

void loop() {
  myStatus.switches = millis();
  //
  sendState();
  //
  delay(2500);
}

void sendState() {
  uint8_t bs[sizeof(myStatus)];
  memcpy(bs, &myStatus, sizeof(myStatus));
  esp_now_send(NULL, bs, sizeof(myStatus));
}
