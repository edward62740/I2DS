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

void managerDeviceTimerCallback(TimerHandle_t managerDeviceTimer)
{
  uint8_t id = (uint32_t)pvTimerGetTimerID(managerDeviceTimer);
  sensorInfoExt[id].alive = false;
  Serial.print("EXPIRED: ");
  Serial.println(id);
  if (uxQueueSpacesAvailable(manager2GuiDeviceIndexQueue) == 0)
  {
    xQueueReset(manager2GuiDeviceIndexQueue);
  }
  xQueueSend(manager2GuiDeviceIndexQueue, (void *)&id, 0);
}
void ipcDeviceUpdateCallback(TimerHandle_t ipcDeviceUpdateTimer)
{
  updateDevice = true;
  Serial.println("Updated");
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
        uint16_t index = tmpInfo.DeviceInfoChangeIndex;
        if (~index & 1)
          sensorInfo[tmpInfo.id].hw = tmpInfo.info.hw;
        if (~index & 2)
          sensorInfo[tmpInfo.id].state = tmpInfo.info.state;
        if (~index & 4)
          sensorInfo[tmpInfo.id].battery_voltage = tmpInfo.info.battery_voltage;
        if (~index & 8)
          sensorInfo[tmpInfo.id].node_type = tmpInfo.info.node_type;
        if (~index & 16)
          sensorInfo[tmpInfo.id].central_id = tmpInfo.info.central_id;
        if (~index & 32)
          sensorInfo[tmpInfo.id].self_id = tmpInfo.info.self_id;
        if (~index & 64)
          sensorInfo[tmpInfo.id].endpoint = tmpInfo.info.endpoint;
        if (~index & 128)
          sensorInfo[tmpInfo.id].trigd = tmpInfo.info.trigd;
        if (~index & 256)
          sensorInfo[tmpInfo.id].rssi = tmpInfo.info.rssi;
        if (~index & 512)
          sensorInfo[tmpInfo.id].lqi = tmpInfo.info.lqi;
        sensorInfoExt[tmpInfo.id].alive = tmpInfo.alive;
        sensorInfoExt[tmpInfo.id].guiUpdatePending = tmpInfo.guiUpdatePending;
        if (uxQueueSpacesAvailable(manager2GuiDeviceIndexQueue) == 0)
        {
          xQueueReset(manager2GuiDeviceIndexQueue);
        }
        xQueueSend(manager2GuiDeviceIndexQueue, (void *)&tmpInfo.id, 0);
      }
    }
    else
    {
      vTaskDelay(1);
    }
  }
}