#include "esphome.h"
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void handleRoot() {
  ESP_LOGD("custom", "request");
  server.send(200, "text/html", F(
    "<!DOCTYPE html><html><head><script src=\"https://kit.fontawesome.com/a076d05399.js\"></script><style>"
    "body {max-width: 500px; margin: auto; margin-top: 300px; margin-bottom:300px; background-color:#3e8e41;}"
    ".button {width: 100%; height: 500px; display: inline-block;  padding: 15px 25px;  font-size: 94px;  cursor: pointer;  text-align: center;  text-decoration: none;  outline: none;  color: #fff;  background-color: #4CAF50;  border: none;  border-radius: 15px;  box-shadow: 0 0px #999;}"
    ".button:hover {background-color: #3e8e41}"
    ".button:active {background-color: #3e8e41;  box-shadow: 0 5px #666;  transform: translateY(4px);}"
    "</style></head><body><button class=button  onclick=\"window.location.href = '/toggle';\"><i class=\"fas fa-holly-berry\"></i></button></body></html>"
  ));
}

void handleToggle() {
  ESP_LOGD("custom", "toggle");
  strobe_on->value() = !strobe_on->value();
  server.send(200, "text/html", F(
    "<!DOCTYPE html><html><head><script src=\"https://kit.fontawesome.com/a076d05399.js\"></script><style>"
    "body {max-width: 500px; margin: auto; margin-top: 300px; margin-bottom:300px; background-color:#3e8e41;}"
    ".button {width: 100%; height: 500px; display: inline-block;  padding: 15px 25px;  font-size: 94px;  cursor: pointer;  text-align: center;  text-decoration: none;  outline: none;  color: #fff;  background-color: #4CAF50;  border: none;  border-radius: 15px;  box-shadow: 0 0px #999;}"
    ".button:hover {background-color: #3e8e41}"
    ".button:active {background-color: #3e8e41;  box-shadow: 0 5px #666;  transform: translateY(4px);}"
    "</style></head><body><button class=button  onclick=\"window.location.href = '/toggle';\"><i class=\"fas fa-holly-berry\"></i></button></body></html>"
  ));
}

void handleNotFound() {
  ESP_LOGD("custom", "not found");
  server.send(404, "text/plain", "not found");
}

class MyWebServer : public Component {
 public:
  void setup() override {
    server.on("/", handleRoot);
    server.on("/toggle", handleToggle);
    server.onNotFound(handleNotFound);
    server.begin();
  }
  void loop() override {
    server.handleClient();
  }
};
