#include "esphome.h"
#include "Wire.h"

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

#define NODES_MAX    16
#define NODES_1ST  0x11

#define POLL_RING  1000
#define POLL_SENSOR 200
#define POLL_SWITCH 900

// Remote device
class RemoteDevice : public PollingComponent {
    uint16_t state[NODES_MAX+1];
  public:
    RemoteDevice() : PollingComponent(POLL_RING) {
        App.register_component(this);
    }
    void setup() override {
        Wire.begin(D2, D1);
        ESP_LOGD("main", "Wire status: %d", Wire.status());
    }
    void loop() override {
      // update node states over i2c
      update();
    }
    void update() override {
      for(int i=0; i<NODES_MAX; i++) {
        uint8_t adr = NODES_1ST + i;
        // request state update
        uint8_t state1 = 0;
        uint8_t state2 = 0;
        int states = 0;
        // request status from address 
        Wire.beginTransmission(adr);
        Wire.requestFrom(uint8_t(adr), uint8_t(3));
        adr = Wire.read();
        state1 = Wire.read();
        state2 = Wire.read();
        Wire.endTransmission();
        //
        states = (state1 << 8) + state2;
        state[adr-NODES_1ST] = states;
        //
        delay(1);
      }  
    }
    bool pin_state(uint8_t adr, uint8_t pin) {
        // cached pin state
        return (state[adr-NODES_1ST] & (1 << (15 - pin)));
    }
    void command(uint8_t adr, uint8_t cmd) {
        // send command to address
        Wire.beginTransmission(adr);
        Wire.write(cmd);
        Wire.endTransmission();
        delay(10);
    }
    // instance
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

