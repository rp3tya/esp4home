#include <EEPROM.h>
#include <Wire.h>

int self_address = 0x20;
int peer_address = 0x16;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  // initialize i2c as slave
  Wire.begin(self_address);
  // define callbacks for i2c communication
  Wire.onReceive(receiveCommand);
  //
  Serial.print("Ready ");
  Serial.println(self_address, HEX);
}

void loop() {
  delay(3000);
  Wire.beginTransmission(0x16);
  int x = Wire.endTransmission();
  Serial.println(x, HEX);
}

// callback for received command
void receiveCommand(int byteCount) {
  Serial.print(byteCount);
  Serial.println(" bytes received");
}
