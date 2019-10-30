#include <EEPROM.h>
#include <Wire.h>

#define DEFAULT_ADDRESS 0x09
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

byte i2c_address = DEFAULT_ADDRESS;
byte eeprom_address = 0x0F;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  //EEPROM.write(eeprom_address, 0x11);

  // initialize all used pins as input
  for (int i=2; i<=17; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  // read address from eeprom
  byte conf_address = EEPROM.read(eeprom_address);
  if (conf_address > 9 and conf_address < 119) {
    i2c_address = conf_address; 
  }
  // initialize i2c as slave
  Wire.begin(i2c_address);
  // define callbacks for i2c communication
  Wire.onReceive(receiveCommand);
  Wire.onRequest(sendState);
  // help
  Serial.print("Ready at 0x");
  Serial.println(i2c_address, HEX);
  Serial.println("Accepting 1-byte commands 0xXY, where:");
  Serial.println("    X - command");
  Serial.println("    Y - parameter");
  Serial.println("Commands:");
  Serial.println("    0x0 - digital write pin to LOW");
  Serial.println("    0x1 - digital write pin to HIGH");
  Serial.println("    0x2 - digital read pin");
  Serial.println("    0x7 - set pin mode to INPUT_PULLUP");
  Serial.println("    0x8 - set pin mode to OUTPUT");
  Serial.println("    0xA - set i2c address to param+0x00");
  Serial.println("    0xB - set i2c address to param+0x10");
  Serial.println("    0xC - set i2c address to param+0x20");
  Serial.println("    0xD - set i2c address to param+0x30");
  Serial.println("    0xE - set i2c address to param+0x40");
  Serial.println("    0xF - set i2c address to param+0x50");
}

uint16_t pin_states = 0;
void loop() {
  int new_state = 0;
  for (int i=2; i<=17; i++) {
    int ps = digitalRead(i);
    new_state = new_state | (ps << (17-i));
  }
  if (new_state != pin_states) {
    pin_states = new_state;
    Serial.println(pin_states, HEX);
  }
  delay(100);
}

// callback for state requests
void sendState() {
  Wire.write(i2c_address);
  Wire.write((pin_states & 0xFF00) >> 8);
  Wire.write((pin_states & 0x00FF) >> 0);
}

// callback for received command
void receiveCommand(int byteCount) {
  Serial.print(byteCount);
  Serial.print(" bytes received ");
  if (byteCount == 0) {
    // empty command = request, state already sent
  } else if (byteCount == 1) {
    // read one byte
    byte msg = Wire.read();
    // convert to comand and param
    byte cmd = (msg & 0xF0) >> 4;
    byte prm = (msg & 0x0F);
    //
    Serial.print("0x");
    Serial.print(msg, HEX);
    Serial.print(" => ");
    //
    byte pin = 0;
    byte state = LOW;
    byte mode = INPUT;
    byte addr = DEFAULT_ADDRESS;
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
        addr = ((cmd - 0xA) << 4) + prm;
        EEPROM.write(eeprom_address, addr);
        Serial.print("saved address 0x");
        Serial.println(addr, HEX);
        Serial.println("reset board!");
        break;
      default:
        Serial.println("unknown command");
        break;
    }
  } else {
    Serial.print("packet too long");
  }
  Serial.println();
  delay(10);
}
