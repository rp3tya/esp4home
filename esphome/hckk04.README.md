# HCKK 04

This device can read the RF signal of [Home HCKK 04](https://www.somogyi.hu/product/kulso-jelado-hcw-21-idojaras-allomashoz-hckk-04-12025) wireless temperature and humidity sensors, which is basically compatible with NEXUS Weather Stations.

## hardware
To build the hardware, you will need:
- ESP board - I prefer ESP32 for this kind of device because settings are restored without issues after power loss 
- 433MHz receiver - for example SRX882

## software
To build the software, you will need:
- ESPHome

Modify `hckk04.yaml` to match your hardware (board, pins, network, etc) and upload to the ESP. Follow the logs, as it will print a House Code when it detects a message from the sensor. Modify `hckk04.yaml` again and set the correct House Code where the custom component is initialized (`hckk04 = new HCKK04Sensor(112)`). Upload again to get the two sensors update correctly.

House Code changes every time the batteries are replaced, and until it is hard-coded, that means firmware upload with the new value.

## to be done

Automatic House Code detection.

Support for multiple sensors.

Some means to update House Code when battery is replaced.
