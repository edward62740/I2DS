#ifndef APP_MANAGER_H
#define APP_MANAGER_H
#include <Arduino.h>

void firebaseTask(void *pvParameters);
void managerTask(void *pvParameters);

typedef struct
{ /* Sensor info */
  uint8_t hw;
  uint8_t state;
  uint32_t battery_voltage;
  uint8_t node_type;
  uint16_t central_id;
  uint16_t self_id;
  uint8_t endpoint;
  uint8_t trigd;
  uint8_t lqi;
  int8_t rssi;
} DeviceInfo;
extern DeviceInfo selfInfo, sensorInfo[30];
extern uint8_t sensorIndex;
uint16_t cmpDeviceInfo(DeviceInfo *s1, DeviceInfo *s2);
typedef struct
{
  DeviceInfo info;
  uint8_t id;
  int16_t touchArea[4]; // x, y, w, h
  bool guiUpdatePending;
  bool ipcResponsePending;
} DeviceInfoExt;
extern DeviceInfoExt selfInfoExt, sensorInfoExt[30];
extern TimerHandle_t ipcDeviceUpdateTimer;
extern QueueHandle_t ipc2ManagerDeviceInfoQueue;
extern bool updateDevice;
#endif  // APP_MANAGER_H