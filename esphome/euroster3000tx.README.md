# euroster3000tx

This device replaces the TX part of a [Euroster 3000TXRX](https://www.euroster.pl/en/produkty/room-thermostats/programmable/euroster-3000txrx/269) room thermostat. It should work with other wireless models that use the same RX unit.

These thermostats have an RX part with a dry contact relay that is connected to the central heating boiler (or electrical heating appliance) and a TX part with built-in temperature sensor and screen/buttons for setup. The two components communicate via 433MHz signals: TX sends an ON or OFF message regularly, while RX switches the relay accordingly.

## requirements
I wanted to replace the thermostat with something that can be controlled from a computer, while not loosing previous features. The TX emulator should:
- continue working standalone if network connection is lost
- change the target temperature according to a predefined schedule (program)
- integrate with Home Assistant as a Climate device
- support multiple temperature sensors

## hardware
To build the hardware, you will need:
- ESP board - I prefer ESP32 for this kind of device because settings are restored without issues after power loss 
- temperature sensor - there is an AM2302 in this configuration, but can be any sensor (even [wireless](https://github.com/rp3tya/esp4home/blob/master/esphome/hckk04.README.md))
- 433MHz transmitter - for example STX882

![Assembled](https://github.com/rp3tya/esp4home/raw/master/esphome/euroster3000tx.jpg)

## software
To build the software, you will need:
- ESPHome
- Home Assistant

Modify `euroster3000tx.yaml` to match your hardware (board, pins, network, etc) and upload it to the ESP. Once the device is flashed and discovered/added in Home Assistant, it must be configured. In Developer Tools, Services look for the `esphome.euroster3000tx_configure_tx` service and call it with the following parameters:
```
rx_on: <ON code>
rx_off: <OFF code>
```

## getting the ON/OFF codes
Initially I used [rtl_433](https://github.com/merbanan/rtl_433) with a DVB [dongle](https://www.rtl-sdr.com/product/rtl-sdr-blog-v3-r820t2-rtl2832u-1ppm-tcxo-sma-software-defined-radio-dongle-only/) to read the signal (see [here](https://github.com/merbanan/rtl_433_tests/tree/master/tests/euroster/3000tx/01) more protocol details). To find your TX ON/OFF message codes, run rtl_433 with

```$ rtl_433 -R 0 -X "n=Euroster_3000TX,m=OOK_MC_ZEROBIT,s=1000,r=4800,bits=32"```

![Euroster TX in RTL](https://github.com/rp3tya/esp4home/raw/master/esphome/euroster3000tx.rtl.png)

Note the number displayed as `data` and the heating status on your thermostat. Now, modify the target temperature on the unit to toggle the heating. Wait for approximately a minute and when the next message is displayed, it should give a different `data` value. These are the ON/OFF codes of your unit. If you live in a crowded area make sure you did not record some neighbour's codes, for example remove the batteries (no messages should be detected by rtl_433 for at least a minute). Do not replace the batteries in the thermostat from this point as it will interfere with the new controller!

Concerned about security? You are right!

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
where `HH` is the hour, `MM` is minute and `TT` is the target temperature. To make it configurable from Lovelace I used these entities in Home Assistant:
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
...
```

Currently there are four set of values configured in `euroster3000tx.yaml`, but more can be added if necessary. Naturally each set needs corresponding entities in Home Assistant if you want to change the schedule from the user interface (otherwise this is optional).

![Daily schedule in Home Assistant](https://github.com/rp3tya/esp4home/raw/master/esphome/euroster3000tx.schedule.png)

After you change the schedule in Home Assistant it must be sent to the device, for example with a script:

```
transfer_schedule:
  alias: Transfer Schedule
  sequence:
    - service: esphome.euroster3000tx_configure_schedule
      data_template:
        weekday1: '{{ states.input_select.hour_wd1.state + states.input_select.minute_wd1.state + states.input_select.target_wd1.state }}'
        weekday2: '{{ states.input_select.hour_wd2.state + states.input_select.minute_wd2.state + states.input_select.target_wd2.state }}'
        weekday3: '{{ states.input_select.hour_wd3.state + states.input_select.minute_wd3.state + states.input_select.target_wd3.state }}'
        weekday4: '{{ states.input_select.hour_wd4.state + states.input_select.minute_wd4.state + states.input_select.target_wd4.state }}'
        weekend1: '{{ states.input_select.hour_we1.state + states.input_select.minute_we1.state + states.input_select.target_we1.state }}'
        weekend2: '{{ states.input_select.hour_we2.state + states.input_select.minute_we2.state + states.input_select.target_we2.state }}'
        weekend3: '{{ states.input_select.hour_we3.state + states.input_select.minute_we3.state + states.input_select.target_we3.state }}'
        weekend4: '{{ states.input_select.hour_we4.state + states.input_select.minute_we4.state + states.input_select.target_we4.state }}'
```

## to be done

The climate entity should not be set manually because on-board automations will reset the target temperature according to schedule. This is a side-effect of making the schedule configurable on the fly, as `on_time` is not templatable.

In case of power outage or restart the ESP needs to get current time from a server, otherwise it can not follow the schedule. Without valid time, it will keep the last target temperature until an NTP server or Home Assistant can be contacted. A custom time component would probably overcome this limitation.

I am planning to add a 433MHz receiver to support multiple external temperature sensors (as in [hckk04](https://github.com/rp3tya/esp4home/blob/master/esphome/hckk04.README.md))


