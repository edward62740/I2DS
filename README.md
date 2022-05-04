# Internal Intruder Detection System (I<sup>2</sup>DS)

## Overview
This is a sub-GHz proprietary security sensor system. The sensors listed below will enter sensing mode once enabled through the [CPN GUI](https://github.com/edward62740/i2ds/blob/master/CPN%20ESP32/src/app_gui.cpp) or the [mobile app](https://github.com/edward62740/i2ds/tree/master/App) . Any detected intrusion will then trigger a FCM background notification on the smartphones of the users with the mobile app installed.
 
 
 ## Sensors
 CPN             |  PIRSN      |  ACSN
:-------------------------:|:-------------------------:|:-------------------------:
<img src="https://github.com/edward62740/i2ds/blob/master/Documentation/ic_cpn.png" alt="CPN" width="200"/><br />This is the main network controller <br> and provides a touch GUI for setting sensors|<img src="https://github.com/edward62740/i2ds/blob/master/Documentation/ic_pirsn.png" alt="PIRSN" width="200"/><br />This is a PIR sensor node for detecting *motion* |  <img src="https://github.com/edward62740/i2ds/blob/master/Documentation/ic_acsn.png" alt="ACSN" width="200"/><br />This is a hall-effect sensor for detecting <br> open/closed doors and windows

## System Structure
![](https://github.com/edward62740/i2ds/blob/master/Documentation/functional.png)
#### Subnetwork
All network management and communication between sensors is restricted to this region. Some management functions are exposed to the Wi-Fi/GUI co-processor via USART.
#### Local Sensor Network
Includes the subnetwork, as well as the CPN Wi-Fi section, which is used to communicate with Firebase RTDB.
#### Mobile App
Used to control the active/inactive state of the sensors, receive status updates.
#### FCM
Used to detect certain changes to the RTDB (i.e sensor triggered), and notify the user even if the app is not running.

## Security
Security is taken seriously in this system.
* The sensor HW is built around the EFR32xG23 series, industry-leading at the time of this release, with PSA 3 level security qualifications. Anti-tamper functions are configured such that the sensor will automatically raise a warning and drop out of the network if tamper is detected.
* Sensor OTA communication is AES encrypted.
* Only necessary state control functions are exposed outside of the subnetwork.
