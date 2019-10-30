#include "esphome.h"
#include "SoftwareSerial.h"
#include "ArduinoJson.h"

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

#define NODES_RX      5
#define NODES_TX      4
#define NODES_BAUD 9600
#define NODES_MAX    16
#define NODES_1ST  0x11

#define POLL_RING 60000
#define POLL_SENSOR 200
#define POLL_SWITCH 200

#define PACK_SIZE 256

// Remote device
class RemoteDevice : public PollingComponent {
    SoftwareSerial mySerial = SoftwareSerial(NODES_RX, NODES_TX);
    StaticJsonDocument<PACK_SIZE> doc;
    uint16_t state[NODES_MAX+1];
    bool msg2send = false;
  public:
    RemoteDevice() : PollingComponent(POLL_RING) {
        App.register_component(this);
    }
    void setup() override {
        mySerial.begin(NODES_BAUD);
    }
    void loop() override {
      // send prepared message
      if (msg2send) {
        serializeMsgPack(doc, mySerial);
        msg2send = false;
      }
      // receive
      if (mySerial.available()) {
        DeserializationError error = deserializeMsgPack(doc, mySerial);
        //
        if (!error) {
          //ESP_LOGD("main", "states");
          for(int i=0; i<NODES_MAX; i++) {
            int adr = NODES_1ST + i;
            if (!doc[String(adr)].isNull()) {
              state[adr-NODES_1ST] = doc[String(adr)];
              //ESP_LOGD("main", "  %d: 0x%04x %d", i, state[i], state[i]);
            }
          }
        }
        // clear message
        doc.clear();
      }
    }
    void update() override {
        // empty message will come back with current states
        msg2send = true;
    }
    bool pin_state(int adr, int pin) {
        // get cached pin state
        return (state[adr-NODES_1ST] & (1 << (15 - pin)));
    }
    void command(uint8_t adr, uint8_t cmd) {
        // add command to message
        doc[String(adr)].add(cmd);
        msg2send = true;
    }

    static RemoteDevice* device() {
		static RemoteDevice dev;
        return &dev;
    }
};

// Remote sensor
class RemoteSensor : public PollingComponent, public BinarySensor {
    RemoteDevice* device;
    int adr = 0;
    int pin = 0;
  public:
    RemoteSensor(int addr, int gpio) : PollingComponent(POLL_SENSOR) {
        device = RemoteDevice::device();
        adr = addr;
        pin = gpio-2;
    }
    void setup() override {
        // set pin mode to input
       device->command(adr, (CMD_MODE_U << 4) + pin);
    }
    void update() override {
        // get current state...
        bool reply = device->pin_state(adr, pin);
        // ...and publish it
        publish_state(reply);
    }
};

// Remote switch
class RemoteSwitch : public PollingComponent, public Switch {
    RemoteDevice* device;
    int adr = 0;
    int pin = 0;
  public:
    RemoteSwitch(int addr, int gpio) : PollingComponent(POLL_SWITCH) {
        device = RemoteDevice::device();
        adr = addr;
        pin = gpio-2;
    }
    void setup() override {
        // set pin mode to output
        device->command(adr, (CMD_MODE_O << 4) + pin);
        // get current state...
        bool reply = device->pin_state(adr, pin);
        // ...and publish it
        publish_state(reply);
    }    
    void update() override {
        // get current state...
        bool reply = device->pin_state(adr, pin);
        // ...and publish it
        publish_state(reply);
    }
    void write_state(bool state) override {
        // set state on remote device
        device->command(adr, ((state ? CMD_WRITE_H : CMD_WRITE_L) << 4) + pin);
        // ...and aknowledge
        publish_state(state);
    }
};

