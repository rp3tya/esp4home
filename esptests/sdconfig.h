#include "esphome.h"
#include <SD.h>

void listFiles() {
    SD.begin(D4);
    File root = SD.open("/");
    while(true) {
     File entry =  root.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     ESP_LOGI("files", " %s", entry.name());
     entry.close();
    }
};

String loadConfig(String prop) {
  String cfg;
  SD.begin(D4);
  File myFile = SD.open("test.txt");
  if (myFile) {
    while (myFile.available()) {
      cfg = myFile.readString();
      if (cfg.indexOf(prop)==0) {
        break;
      }
    }
    myFile.close();
  } else {
    ESP_LOGE("sd", "read error");
  }
  return cfg;
}

void saveConfig(String cfg) {
  SD.begin(D4);
  SD.remove("test.txt");
  File myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(cfg);
    myFile.close();
  } else {
    ESP_LOGE("sd", "write error");
  }
}

