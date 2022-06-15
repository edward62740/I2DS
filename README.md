# Internal Intruder Detection System (I<sup>2</sup>DS) [IN PROGRESS]

## Overview
This is a sub-GHz proprietary security sensor system. The sensors listed below will enter sensing mode once enabled through the [CPN GUI](https://github.com/edward62740/i2ds/blob/master/CPN%20ESP32/src/app_gui.cpp) or the [mobile app](https://github.com/edward62740/i2ds/tree/master/App). Any detected intrusion will then trigger a [FCM](https://github.com/edward62740/i2ds-fcm) background notification on the smartphones of the users with the mobile app installed.
Used in conjunction with [WMNS](https://github.com/edward62740/Wireless-Mesh-Network-System) for integrated data collection and security.

## System Structure
![](https://github.com/edward62740/i2ds/blob/master/Documentation/functional.png)
#### Subnetwork
All network management and communication between sensors is restricted to this region. Some management functions are exposed to the Wi-Fi/GUI co-processor via USART.
#### Local Sensor Network
Includes the subnetwork, as well as the CPN Wi-Fi section, which is used to communicate with Firebase RTDB.
#### Mobile App
Used to control the active/inactive state of the sensors, receive status updates.<br><br>
<img src="https://github.com/edward62740/i2ds/blob/master/Documentation/App-main.jpg" alt="App" width="200"/>
<img src="https://github.com/edward62740/i2ds/blob/master/Documentation/App-settings.jpg" alt="App" width="200"/>
<img src="https://github.com/edward62740/i2ds/blob/master/Documentation/App-errors.jpg" alt="App" width="200"/>
#### FCM
Used to detect certain changes to the RTDB (i.e sensor triggered), and notify the user even if the app is not running.

 
 ## Sensors
 CPN             |  PIRSN      |  ACSN
:-------------------------:|:-------------------------:|:-------------------------:
<img src="https://github.com/edward62740/i2ds/blob/master/Documentation/ic_cpn.png" alt="CPN" width="200"/><br />This is the main network controller <br> and provides a touch GUI for setting sensors|<img src="https://github.com/edward62740/i2ds/blob/master/Documentation/ic_pirsn.png" alt="PIRSN" width="200"/><br />This is a PIR sensor node for detecting *motion* |  <img src="https://github.com/edward62740/i2ds/blob/master/Documentation/ic_acsn.png" alt="ACSN" width="200"/><br />This is a hall-effect sensor for detecting <br> open/closed doors and windows

## Important Information
### PIRSN
* This sensor uses EKMB1110112 PIR sensor to detect motion up to a distance of 7m (indoors).
* Typical 5.5uA in sleep, avg 19uA with 2s polling and 5s reporting interval.
<img src="https://github.com/edward62740/I2DS/blob/master/Documentation/prod-pirsn.jpg" alt="pirsn" width="200"/>


### ACSN
* This sensor uses the SM353LT hall effect sensor to detect when the sensor's magnet moves away (door opens).
* Typical 5.3uA in sleep, avg 19uA with 2s polling and 5s reporting interval.
<img src="https://github.com/edward62740/I2DS/blob/master/Documentation/prod-acsn.jpg" alt="pirsn" width="200"/>

### CPN
* This device contains two microcontrollers, for the sensor network [(EFR32xG23)](https://www.silabs.com/wireless/proprietary/efr32fg23-sub-ghz-wireless-soc) and Wi-Fi/GUI (ESP32). Sensor network functions are exposed to the Wi-Fi processor via USART. While there is significant traffic, in effect the only change that can be made to the sensor network is to activate/inactivate the sensors. Doing so also requires the relevant Firebase auth credentials. This provides some protection from attacks based on injecting false data into the system to incapacitate the sensor network.
* This device also provides internal battery backup with FCM notification when triggered. It is assumed that physical security beyond debug lock, encryption etc. is not crucial as the sensors would have already triggered warnings before the CPN can be tampered with. This functionality is mostly to keep the system afloat during a power outage (but this must be used with a backup 4G network or other such systems).
### Overall System
* From testing, there was a net delay of 3-4s from sensor triggered to receiving FCM notification on client smartphone. Results will vary based on Internet speeds.
* Sensors are capable of receiving messages to implement the aforementioned enabling/disabling feature. This is necessary to prevent constant triggering of the sensor when not in use, and is much more energy efficient than letting the sensor trigger all the time and filter on the coordinator side.
* The subnetwork communicates in the 915MHz ISM band, but a simple hardware swap of the chip antenna (W3211 -> W3214) allows for reconfiguration to 868MHz band. RF matching is optimized for both. Modulation/frequency is set to OQPSK 500kbps, to increase operational range set to DSSS with lower bitrate, enable FEC.

## Security
* The sensor HW is built around the [EFR32xG23](https://www.silabs.com/wireless/proprietary/efr32fg23-sub-ghz-wireless-soc) series, industry-leading at the time of this release, with PSA 3 level security qualifications. Anti-tamper functions are configured such that the sensor will automatically raise a warning and drop out of the network if tamper is detected.
* Sensor OTA communication is AES encrypted with [Mbed TLS](https://github.com/Mbed-TLS/mbedtls).
* Only necessary state control functions are exposed outside of the subnetwork.
* Parts of this project are not published due to security concerns. Please contact the developer for more information.

## Upkeep
* Estimated $0.00 cost from Firebase cloud functions for triggering [FCM](https://github.com/edward62740/i2ds-fcm), unless triggered more than 2 million times/month for some reason.
* More than 2 years battery life for sensors, using low-cost CR2450 cells.
* Electricity costs from CPN. Consumes less power than a night-light.


