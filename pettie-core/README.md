# Pettie

***


## Project info
This is core part of the robot where all magic happen.

All project has been written using bare ESP-IDF SDK from *Eperesif*.

### Used Hardware:
- *ESP32-D0WD* as ESP32-DevKitV1 (with no PSRAM chip)
- 8 MG90S Servos
- Quadruped Q1 chassis
- VL53l0x as a distance sensor (TBD)


### Used libraries:
- [Nematoduino](https://github.com/nategri/nematoduino.git "Nematoduino")
 
 
Libraries above are modified to work with ESP-IDF.
 

## Building the project

To be able to build firmware you will be needed to install:
 - *ESP-IDF SDK* as minimum release/5.1 branch
 - VSCode
 - Installed Git, Pip and Python 3.10 or higher
 
ESP-IDF might be installed as extension if VSCode.


### Observing Neurons(cells)

That might be funny, but sometimes you may be interested in what's inside it's brain.

In that case you'll need a neuron-viewer, MQTT broker and Wi-Fi.

#### Configuring Wi-Fi

Settings of the Wi-Fi is located in wireless\wireless_conf.h file.
Just edit lines below with your settings:
```
#define WIFI_SSID_NAME      "YOUR_SSID_NAME"
#define WIFI_SSID_PASSWORD  "YOUR_SSID_PASSWORD"
```
Please, note what settings MUST be in quotes!

And, yes i know about menuconfig and what this has to be done up there.
This software allow to use multiple Wi-Fi access points ;)

#### Configuring MQTT

Settings of the MQTT is located in mqtt\mqtt_config.h file.
Just edit lines below with your settings:
```
#define MQTT_SERVER_ADDR "mqtt://127.0.0.1" 
```
Please, note what settings MUST be in quotes!

Replace localhost IP with your actual broker IP.
