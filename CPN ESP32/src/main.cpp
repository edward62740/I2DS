#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

void ipcTask(void *pvParameters);
void firebaseTask(void *pvParameters);
void displayTask(void *pvParameters);
bool ipcParser(char *buffer, size_t len);

/* Pin Configuration */
#define XPT_CS 16
#define XPT_IRQ 17
#define TFT_CS 15
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_RST 4
#define TFT_MISO 35
#define IPC_TX 22
#define IPC_RX 19
#define IPC_HWFC_CTS 21
#define IPC_HWFC_RTS 25
#define LED_1 26
#define LED_2 27

/* Defines */
#define EUSART_IPC_BAUD 115200
#define SPI_DISPLAY_FREQ 48000000

/* Task Handles */
static TaskHandle_t ipc;
static TaskHandle_t firebase;
static TaskHandle_t display;

/* Objects */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
XPT2046 touch(16, 17);
static uint16_t prev_x = 0xffff, prev_y = 0xffff;

typedef enum
{                    /* Sensor state byte */
  S_ACTIVE = 0x05,   // Sensor element active
  S_INACTIVE,        // Sensor element inactive
  S_FAULT_HW = 0xCA, // Hardware fault detected
  S_FAULT_OPN,       // Operational fault detected
  S_ALERTING,        // Sensor element triggering
} sensor_state_t;

typedef enum
{                /* Hardware identification byte */
  HW_CPN = 0x88, // Control Panel Node (Coordinator)
  HW_PIRSN,      // PIR Sensor Node (Sensor)
  HW_ACSN,       // Access Control Sensor Node (Sensor)
} device_hw_t;

/* IPC Information */

typedef enum
{                   /* IPC message identification byte */
  IPC_CHANGE,       // (C -> EXT) notify change of information
  IPC_LIST,         // (C -> EXT) send list of connected sensors and associated information
  IPC_REQUEST,      // (C <- EXT) request sensor change state
  IPC_REQUEST_ACK,  // (C -> EXT) ack IPC_REQUEST
  IPC_REQUEST_DONE, // (C -> EXT) finished IPC_REQUEST
  IPC_LIST_CTS,     // (C <- EXT) ack IPC_CHANGE and request IPC_LIST
  IPC_ERR,
} ipc_message_pid_t;
uint8_t IPC_START = 0xAF;
uint8_t IPC_END = 0xAC;
char ipc_recv_buffer[255];
uint8_t ipc_get_list[3] = {IPC_START, (uint8_t)IPC_LIST_CTS, IPC_END};
uint8_t ipc_set_sensor_state[6] = {IPC_START, (uint8_t)IPC_REQUEST, 0x00, 0x00, 0x00, IPC_END};
bool ipc_update_info = false;
bool ipc_awaiting_reply = false;

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
DeviceInfo selfInfo, sensorInfo[30];
uint8_t sensorIndex = 0;
uint16_t cmpDeviceInfo(DeviceInfo *s1, DeviceInfo *s2);
typedef struct
{
  int16_t touchArea[4]; // x, y, w, h
  bool updatePending;
  bool responsePending;
} DeviceInfoExt;
DeviceInfoExt sensorInfoExt[30];

void setup()
{
  tft.begin();
  touch.begin(240, 320); // Must be done before setting rotation
  tft.fillScreen(ILI9341_BLACK);
  touch.setRotation(touch.ROT180);

  // Replace these for your screen module
  touch.setCalibration(209, 1759, 1775, 273);
  // read diagnostics (optional but can help debug problems)

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, IPC_RX, IPC_TX);

  tft.setCursor(10, 15);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(ILI9341_ORANGE);
  tft.print("I2DS Control Panel");

  xTaskCreatePinnedToCore( // Use xTaskCreate()
      ipcTask,             // Function to be called
      "IPC Task",          // Name of task
      32768,               // Stack size
      NULL,                // Parameter to pass
      1,                   // Task priority
      &ipc,                // Task handle
      1);                  // CPU Core

  xTaskCreatePinnedToCore( // Use xTaskCreate()
      firebaseTask,        // Function to be called
      "Firebase Task",     // Name of task
      32768,               // Stack size
      NULL,                // Parameter to pass
      1,                   // Task priority
      &firebase,           // Task handle
      0);                  // CPU Core

  xTaskCreatePinnedToCore( // Use xTaskCreate()
      displayTask,         // Function to be called
      "Display Task",      // Name of task
      32768,               // Stack size
      NULL,                // Parameter to pass
      0,                   // Task priority
      &display,            // Task handle
      0);                  // CPU Core
  vTaskDelete(NULL);
}

void ipcTask(void *pvParameters)
{
  while (1)
  {
    if (Serial1.available())
    {
      size_t ret = 0;
      ret = Serial1.readBytesUntil(IPC_END, (char *)ipc_recv_buffer, sizeof(ipc_recv_buffer));
      // Serial.write(ipc_recv_buffer, ret);
      ipcParser(ipc_recv_buffer, ret);
    }
    delay(1);
  }
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

void displayTask(void *pvParameters)
{
  while (1)
  {
    if (touch.isTouching() & !ipc_awaiting_reply)
    {
      uint16_t x, y;
      touch.getPosition(x, y);
      for (uint8_t i = 0; i < sensorIndex; i++)
      {
        if ((int16_t)x > sensorInfoExt[i].touchArea[0] && (int16_t)x < (sensorInfoExt[i].touchArea[0] + sensorInfoExt[i].touchArea[2]) &&
            (int16_t)y > sensorInfoExt[i].touchArea[1] && (int16_t)y < (sensorInfoExt[i].touchArea[1] + sensorInfoExt[i].touchArea[3]))
          tft.drawRoundRect(sensorInfoExt[i].touchArea[0] - 1, sensorInfoExt[i].touchArea[1] - 1, sensorInfoExt[i].touchArea[2] + 2, sensorInfoExt[i].touchArea[3] + 2, 5, ILI9341_YELLOW);
        ipc_set_sensor_state[2] = 0xFF & (uint8_t)sensorInfo[i].self_id >> 8;
        ipc_set_sensor_state[3] = 0xFF & (uint8_t)sensorInfo[i].self_id;
        if (sensorInfo[i].state == (uint8_t)S_INACTIVE)
          ipc_set_sensor_state[4] = 0xFF & (uint8_t)S_ACTIVE;
        else
          ipc_set_sensor_state[4] = 0xFF & (uint8_t)S_INACTIVE;
        Serial1.write(ipc_set_sensor_state, sizeof(ipc_set_sensor_state));
        sensorInfoExt[i].responsePending = true;
        ipc_awaiting_reply = true;
      }
    }
    if (ipc_update_info)
    {
      
      for (uint8_t i = 0; i < sensorIndex; i++)
      {
        if (sensorInfoExt[i].updatePending)
        {
          Serial.println("DISPLAY");
          tft.fillRoundRect(10 + ((i % 2) * 120), 50 + ((i / 2) * 120), 100, 100, 5, ILI9341_DARKGREEN);
          sensorInfoExt[i].touchArea[0] = 10 + ((i % 2) * 120);
          sensorInfoExt[i].touchArea[1] = 50 + ((i / 2) * 120);
          sensorInfoExt[i].touchArea[2] = 100;
          sensorInfoExt[i].touchArea[3] = 100;

          tft.setFont(&FreeSansBold9pt7b);
          tft.setTextColor(ILI9341_BLUE);
          tft.setCursor(15 + ((i % 2) * 120), 65 + ((i / 2) * 120));
          switch (sensorInfo[i].hw)
          {
          case HW_PIRSN:
            tft.print("PIRSN");
            break;
          case HW_ACSN:
            tft.print("ACSN");
            break;
          default:
            tft.print("?");
            break;
          }
          tft.setTextColor(ILI9341_WHITE);
          tft.setCursor(15 + ((i % 2) * 120), 95 + ((i / 2) * 120));
          tft.print(sensorInfo[i].self_id);
          tft.setCursor(15 + ((i % 2) * 120), 125 + ((i / 2) * 120));
          tft.print(sensorInfo[i].state);
          // tft.setCursor(50, 50 + i * 30);
          // tft.setTextColor(ILI9341_CYAN);
          // tft.print("PIRSN");
          sensorInfoExt[i].updatePending = false;
        }
      }
      ipc_update_info = false;
    }
    delay(50);
  }
}

void loop()
{
}

bool ipcParser(char *buffer, size_t len)
{
  if (len > 255)
    return false;

  char tmpBuf[len];
  memcpy(tmpBuf, buffer, len * sizeof(char));
  // Serial.write(tmpBuf, len);
  if (tmpBuf[0] != (char)IPC_START)
    return false;
  switch (tmpBuf[1])
  {
  case IPC_LIST:
  {
    for (uint8_t i = 0; i < (uint8_t)(tmpBuf[2]); i++)
    {
      uint8_t tmpIndex = 0;
      DeviceInfo tmpInfo;
      tmpInfo.hw = (uint8_t)tmpBuf[3 + (i * 15)];
      tmpInfo.state = (uint8_t)tmpBuf[4 + (i * 15)];
      tmpInfo.battery_voltage = (uint32_t)((tmpBuf[5 + (i * 15)] << 24) | (tmpBuf[6 + (i * 15)] << 16) | (tmpBuf[7 + (i * 15)] << 8) | tmpBuf[8 + (i * 15)]);
      tmpInfo.node_type = (uint8_t)tmpBuf[9 + (i * 15)];
      tmpInfo.central_id = (uint16_t)((tmpBuf[10 + (i * 15)] << 8) | tmpBuf[11 + (i * 15)]);
      tmpInfo.self_id = (uint16_t)((tmpBuf[12 + (i * 15)] << 8) | tmpBuf[13 + (i * 15)]);
      tmpInfo.endpoint = (uint8_t)tmpBuf[14 + (i * 15)];
      tmpInfo.trigd = (uint8_t)tmpBuf[15 + (i * 15)];
      tmpInfo.rssi = (int8_t)tmpBuf[16 + (i * 15)];
      tmpInfo.lqi = (uint8_t)tmpBuf[17 + (i * 15)];
      for (uint8_t i = 0; i < sensorIndex; i++)
      {
        if (sensorInfo[i].self_id == tmpInfo.self_id)
        {
          Serial.println((uint16_t)cmpDeviceInfo(&tmpInfo, &sensorInfo[i]));
          if (cmpDeviceInfo(&tmpInfo, &sensorInfo[i]) != 1023)
          {
            sensorInfo[i].hw = tmpInfo.hw;
            sensorInfo[i].state = tmpInfo.state;
            sensorInfo[i].battery_voltage = tmpInfo.battery_voltage;
            sensorInfo[i].node_type = tmpInfo.node_type;
            sensorInfo[i].central_id = tmpInfo.central_id;
            sensorInfo[i].self_id = tmpInfo.self_id;
            sensorInfo[i].endpoint = tmpInfo.endpoint;
            sensorInfo[i].trigd = tmpInfo.trigd;
            sensorInfo[i].rssi = tmpInfo.rssi;
            sensorInfo[i].lqi = tmpInfo.lqi;
            sensorInfoExt[i].updatePending = true;
          }
          break;
        }
        else
          tmpIndex++;
      }
      if (tmpIndex == sensorIndex)
      {
        sensorInfo[i].hw = tmpInfo.hw;
        sensorInfo[i].state = tmpInfo.state;
        sensorInfo[i].battery_voltage = tmpInfo.battery_voltage;
        sensorInfo[i].node_type = tmpInfo.node_type;
        sensorInfo[i].central_id = tmpInfo.central_id;
        sensorInfo[i].self_id = tmpInfo.self_id;
        sensorInfo[i].endpoint = tmpInfo.endpoint;
        sensorInfo[i].trigd = tmpInfo.trigd;
        sensorInfo[i].rssi = tmpInfo.rssi;
        sensorInfo[i].lqi = tmpInfo.lqi;
        sensorInfoExt[i].updatePending = true;
        sensorInfoExt[i].responsePending = false;
        sensorIndex++;
        Serial.println("ADDED");
        //  Serial1.write(ipc_get_list, sizeof(ipc_get_list));
      }

      sensorIndex = tmpBuf[2];
    }
    ipc_update_info = true;
    break;
  }
  case IPC_CHANGE:
  {
    Serial.println("NEW");
    uint16_t id = (tmpBuf[2] << 8) | tmpBuf[3];
    uint8_t tmpIndex = 0;
    for (uint8_t i = 0; i < sensorIndex; i++)
    {
      if (sensorInfo[i].self_id == id)
      {
        // sensorInfo[id].state = (uint8_t)tmpBuf[4];
        //***** trigger firebase task *****//
        break;
      }
      else
        tmpIndex++;
    }
    if (tmpIndex == sensorIndex)
    {
      if ((uint8_t)tmpBuf[5] == 2)
      {
        Serial1.write(ipc_get_list, sizeof(ipc_get_list));
      }
    }

    break;
  }
  case IPC_REQUEST_DONE:
  {
    if (ipc_awaiting_reply)
    {
      uint16_t id = ((tmpBuf[2] << 8) | tmpBuf[3]);
      for (uint8_t i = 0; i < sensorIndex; i++)
      {
        if (sensorInfo[i].self_id == id)
        {
          sensorInfoExt[i].updatePending = true;
          sensorInfoExt[sensorIndex].responsePending = false;
          
          ipc_awaiting_reply = false;
          ipc_update_info = true;
          Serial.println(id);
          break;
        }
      }
    }
    break;
  }
  }
  return true;
}
uint16_t cmpDeviceInfo(DeviceInfo *s1, DeviceInfo *s2)
{
  uint16_t ret = 0;
  ret += (s1->hw == s2->hw) ? 1 : 0;
  ret += (s1->state == s2->state) ? 2 : 0;
  ret += (s1->battery_voltage == s2->battery_voltage) ? 4 : 0;
  ret += (s1->node_type == s2->node_type) ? 8 : 0;
  ret += (s1->central_id == s2->central_id) ? 16 : 0;
  ret += (s1->self_id == s2->self_id) ? 32 : 0;
  ret += (s1->endpoint == s2->endpoint) ? 64 : 0;
  ret += (s1->trigd == s2->trigd) ? 128 : 0;
  ret += (s1->rssi == s2->rssi) ? 256 : 0;
  ret += (s1->lqi == s2->lqi) ? 512 : 0;
  return ret;
}