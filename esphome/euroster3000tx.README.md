# euroster3000tx

This node replaces the TX part of a [Euroster 3000TXRX](https://www.euroster.pl/en/produkty/room-thermostats/programmable/euroster-3000txrx/269) room thermostat. It should work with other wireless models that use the same RX unit.

These thermostats have an RX part with a dry contact relay that is connected to the central heating boiler (or electrical heating appliance) and a TX part with built-in temperature sensor and screen/buttons for setup. The two components communicate via 433MHz signals: TX sends an ON or OFF message regularly, while RX switches the relay accordingly.

## requirements
I wanted to replace the thermostat with something that can be controlled from a computer, while not loosing previous features. The TX emulator should:
- work standalone as long as power is available (without a network connection)
- change the target temperature according to a predefined schedule (program)
- integrate with Home Assistant as a Climate device
- support external sensors, too

## hardware
To build the hardware, you will need:
- ESP32 board
- temperature sensor
- 433MHz transmitter

## software
To build the software, you will need:
- ESPHome
- Home Assistant

Once the device is flashed and discovered in Home Assistant, it must be configured. In Developer Tools, Services look for the `esphome.euroster3000tx_configure_tx` service and call it with the following parameters:
```
rx_on: <123456>
rx_off: <012345>
```

## getting the ON/OFF codes
Initially I used [rtl_433](https://github.com/merbanan/rtl_433) with a DVB [dongle](https://www.rtl-sdr.com/product/rtl-sdr-blog-v3-r820t2-rtl2832u-1ppm-tcxo-sma-software-defined-radio-dongle-only/) to read the signal (see [here](https://github.com/merbanan/rtl_433_tests/tree/master/tests/euroster/3000tx/01) more protocol details).

![Euroster TX in RTL](https://github.com/rp3tya/esp4home/raw/master/esphome/euroster3000tx.rtl.png)

To find TX ON/OFF message codes, run rtl_433 with

```$ rtl_433 -R 0 -X "n=Euroster_3000TX,m=OOK_MC_ZEROBIT,s=1000,r=4800,bits=32"```

Note the number displayed as `data` and the heating status on your thermostat. Now, modify the target temperature on the unit to change the heating status. Wait for approximately a minute and when the next message is displayed, it should contain a different `data` value. These are the ON/OFF codes of your unit. If you live in a crowded area make sure you did not record some neighbour's codes, for example by removing the batteries (no messages should be detected by rtl_433 for at least a minute).

## daily schedule
Use the `esphome.euroster3000tx_configure_schedule` service to change the default schedule. Parameters:
```
weekday1: <HHMMTT>
weekday2: <HHMMTT>
weekday3: <HHMMTT>
weekday4: <HHMMTT>
weekend1: <HHMMTT>
weekend2: <HHMMTT>
weekend3: <HHMMTT>
weekend4: <HHMMTT>
```
where `HH` is the hour, `MM` is minute and `TT` is the target temperature. To make it configurable from Lovelace I used similar entities in Home Assistant:
```
input_select:
  hour_wd1:
    name: Hour1
    icon: mdi:clock-outline
    options: ["00","01","02","03","04","05","06","07","08","09","10","11","12","13","14","15","16","17","18","19","20","21","22","23"]
  minute_wd1:
    name: Minute1
    icon: mdi:circle-slice-3
    options: ["00","05","10","15","20","25","30","35","40","45","50","55"]
  target_wd1:
    name: Target1
    icon: mdi:thermometer
    options: ["16","17","18","19","20","21","22","23","24"]
```
![Daily schedule in Home Assistant](https://github.com/rp3tya/esp4home/raw/master/esphome/euroster3000tx.schedule.png)

## to be done
In case of power outage or restart the ESP needs to get current time from a server, otherwise it will not follow the schedule. Without knowing the time, it will keep the last target temperature until a time source can be contacted.

I am planning to add a 433MHz receiver to support multiple external temperature sensors (as in [hckk04](https://github.com/rp3tya/esp4home/blob/master/esphome/hckk04.README.md))


