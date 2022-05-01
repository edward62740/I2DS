#ifndef APP_MANAGER_H
#define APP_MANAGER_H
#include <Arduino.h>

void managerDeviceTimerCallback(TimerHandle_t managerDeviceTimer);
void firebaseTask(void *pvParameters);
void managerTask(void *pvParameters);
typedef uint32_t err_count_t;
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
  bool alive;
  uint16_t DeviceInfoChangeIndex;
} DeviceInfoExt;

extern DeviceInfoExt selfInfoExt, sensorInfoExt[30];
extern TimerHandle_t ipcDeviceUpdateTimer;
extern QueueHandle_t ipc2ManagerDeviceInfoQueue;
extern bool updateDevice;

typedef struct
{
  err_count_t IPC_REQUEST_SEND_NOACK;
  err_count_t IPC_REQUEST_SEND_FAIL;
  err_count_t IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW;
  err_count_t IPC_QUEUE_SEND_DEVICEINFO_FAIL;
  err_count_t IPC_CHANGE_INVALID;
  err_count_t IPC_CHANGE_INDEX_OUT_OF_BOUNDS;
  err_count_t MANAGER_QUEUE_SEND_DEVICEINDEX_OVERFLOW;
  err_count_t MANAGER_QUEUE_SEND_DEVICEINDEX_FAIL;
  err_count_t MANAGER_QUEUE_RECEIVE_OUT_OF_BOUNDS;
  err_count_t FIREBASE_AUTH_ERR;
  err_count_t FIREBASE_REGULAR_UPDATE_FAIL;
  err_count_t FIREBASE_FORCED_UPDATE_FAIL;
  err_count_t FIREBASE_ERR_QUEUE_OVERFLOW;
  err_count_t FIREBASE_NETWORK_FAIL;
} ErrCount;
extern ErrCount err_count;
#endif // APP_MANAGER_H