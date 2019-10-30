#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define DEFAULT_ADDRESS 0x00
#define CMD_WRITE_L 0x0
#define CMD_WRITE_H 0x1
#define CMD_READ    0x2
#define CMD_MODE_I  0x6
#define CMD_MODE_U  0x7
#define CMD_MODE_O  0x8
#define CMD_ADDR_A  0xA
#define CMD_ADDR_B  0xB
#define CMD_ADDR_C  0xC
#define CMD_ADDR_D  0xD
#define CMD_ADDR_E  0xE
#define CMD_ADDR_F  0xF

byte eeprom_address = 0x0F;
byte serial_address = DEFAULT_ADDRESS;
String string_address = String(serial_address);
//                      RX, TX
SoftwareSerial mySerial(18, 19);
StaticJsonDocument<256> doc;
// 
uint16_t pin_states = 0;

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);
  while (!Serial);
  // read uart address from eeprom
  byte conf_address = EEPROM.read(eeprom_address);
  if (conf_address > 0 and conf_address < 64) {
    serial_address = conf_address;
    string_address = String(serial_address);
  }
  Serial.print("ready #");
  Serial.println(serial_address);
  // initialize all used pins as input
  for (int i=2; i<=17; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  // initialize uart communication
  mySerial.begin(9600);
}

void loop() {
  // look for new packets
  while (mySerial.available()) {
    DeserializationError error = deserializeMsgPack(doc, mySerial);
    // analyze packet
    if (error) {
      Serial.println(error.c_str());
    } else {
      Serial.print("IN:  ");
      serializeJson(doc, Serial);
      Serial.println();
      // any commands for us?
      if (!doc[string_address].isNull()) {
        JsonArray msgs = doc[string_address];
        for (int i=0; i<msgs.size(); i++) {
          byte msg = msgs.getElement(i);
          byte cmd = (msg & 0xF0) >> 4;
          byte prm = (msg & 0x0F);
          // execute command
          receiveCommand(cmd, prm);
        }
        // update states before passing on message
        uint16_t new_states = 0;
        for (int i=2; i<=17; i++) {
          int ps = digitalRead(i);
          new_states = new_states | (ps << (17-i));
        }
        pin_states = new_states;
      }
      // pass on with our state
      doc[string_address] = pin_states;
      serializeMsgPack(doc, mySerial);
      //
      Serial.print("OUT: ");
      serializeJson(doc, Serial);
      Serial.println();
    }
  }
  // minimal sleep
  delay(10);
  // read pin states
  uint16_t new_states = 0;
  for (int i=2; i<=17; i++) {
    int ps = digitalRead(i);
    new_states = new_states | (ps << (17-i));
  }
  // dispatch state if there are changes
  if (new_states != pin_states) {
    pin_states = new_states;
    // start new message
    doc.clear();
    doc[string_address] = pin_states;
    serializeMsgPack(doc, mySerial);
    //
    Serial.print("GEN: ");
    serializeJson(doc, Serial);
    Serial.println();
  }
  //
}

// callback for received command
void receiveCommand(byte cmd, byte prm) {
  byte pin = 0;
  byte state = 0;
  byte mode = 0;
  // interpret command
  switch (cmd) {
    case CMD_WRITE_L:
    case CMD_WRITE_H:
      pin = prm + 2;
      state = (cmd == CMD_WRITE_H) ? HIGH : LOW;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, state);
      Serial.print("wrote pin ");
      Serial.print(pin);
      Serial.print(" state to ");
      Serial.println(state);
      break;
    case CMD_READ:
      pin = prm + 2;
      state = digitalRead(pin);
      Serial.print("read pin ");
      Serial.print(pin);
      Serial.print(" state ");
      Serial.print(state);
      Serial.println();
      break;
    case CMD_MODE_I:
    case CMD_MODE_U:
    case CMD_MODE_O:
      pin = prm + 2;
      mode = (cmd == CMD_MODE_O) ? OUTPUT : (cmd == CMD_MODE_I) ? INPUT : INPUT_PULLUP;
      pinMode(pin, mode);
      Serial.print("set pin ");
      Serial.print(pin);
      Serial.print(" mode to ");
      Serial.println(mode);
      break;
    case CMD_ADDR_A:
    case CMD_ADDR_B:
    case CMD_ADDR_C:
    case CMD_ADDR_D:
    case CMD_ADDR_E:
    case CMD_ADDR_F:
      serial_address = (prm % 16);
      EEPROM.write(eeprom_address, serial_address);
      string_address = String(serial_address);
      Serial.print("saved address ");
      Serial.println(serial_address);
      break;
    default:
      Serial.println("unknown command");
      break;
  }
}

