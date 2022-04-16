#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include "app_common.h"

DeviceInfo selfInfo, sensorInfo[30];
uint8_t sensorIndex = 0;
DeviceInfoExt selfInfoExt, sensorInfoExt[30];
QueueHandle_t ipc2ManagerDeviceInfoQueue;
TimerHandle_t ipcDeviceUpdateTimer;
bool updateDevice = false;
void ipcDeviceUpdateCallback(TimerHandle_t ipcDeviceUpdateTimer)
{
  updateDevice = true;
}

void firebaseTask(void *pvParameters)
{
  Serial1.write(ipc_get_list, sizeof(ipc_get_list));
  while (1)
  {
    //  Serial1.write(ipc_get_list, sizeof(ipc_get_list));
    delay(10);

    /*if (touch.isTouching())
    {

      uint16_t x, y;
      touch.getPosition(x, y);
      Serial.println(x);
      if (prev_x == 0xffff)
      {
        tft.drawPixel(x, y, 0x001f);
      }
      else
      {
        tft.drawLine(prev_x, prev_y, x, y, 0x001f);
      }
      prev_x = x;
      prev_y = y;
    }
    else
    {
      prev_x = prev_y = 0xffff;
    }*/
  }
}

void managerTask(void *pvParameters)
{
  ipcDeviceUpdateTimer = xTimerCreate("ipcUpdate", 5000, pdTRUE, (void *)0, ipcDeviceUpdateCallback);
  xTimerStart(ipcDeviceUpdateTimer, 0);
  while (1)
  {
    if (uxQueueMessagesWaiting(ipc2ManagerDeviceInfoQueue) > 0)
    {
      DeviceInfoExt tmpInfo;
      if (xQueueReceive(ipc2ManagerDeviceInfoQueue, (void *)&tmpInfo, 1) == pdPASS)
      {
        sensorInfo[tmpInfo.id].hw = tmpInfo.info.hw;
        sensorInfo[tmpInfo.id].state = tmpInfo.info.state;
        sensorInfo[tmpInfo.id].battery_voltage = tmpInfo.info.battery_voltage;
        sensorInfo[tmpInfo.id].node_type = tmpInfo.info.node_type;
        sensorInfo[tmpInfo.id].central_id = tmpInfo.info.central_id;
        sensorInfo[tmpInfo.id].self_id = tmpInfo.info.self_id;
        sensorInfo[tmpInfo.id].endpoint = tmpInfo.info.endpoint;
        sensorInfo[tmpInfo.id].trigd = tmpInfo.info.trigd;
        sensorInfo[tmpInfo.id].rssi = tmpInfo.info.rssi;
        sensorInfo[tmpInfo.id].lqi = tmpInfo.info.lqi;
        sensorInfoExt[tmpInfo.id].guiUpdatePending = tmpInfo.guiUpdatePending;
        Serial.println(tmpInfo.id);
      }
    }
    vTaskDelay(10);
  }
}