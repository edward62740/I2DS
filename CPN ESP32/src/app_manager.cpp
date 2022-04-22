#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include "app_common.h"
#include <FirebaseESP32.h>
#include "WiFi.h"
// Provide the token generation process info.
#include <addons/TokenHelper.h>

DeviceInfo selfInfo, sensorInfo[30];
uint8_t sensorIndex = 0;
DeviceInfoExt selfInfoExt, sensorInfoExt[30];
QueueHandle_t ipc2ManagerDeviceInfoQueue;
TimerHandle_t ipcDeviceUpdateTimer;
String devicename = "I2DS CONTROL PANEL";
// Define the Firebase Data object
FirebaseData fbdo;
// Define the FirebaseAuth data for authentication data
FirebaseAuth auth;
// Define the FirebaseConfig data for config data
FirebaseConfig config;
// Assign the project host and api key (required)

bool updateDevice = false;
bool FLAGfirebaseRegularUpdate = true;
bool FLAGfirebaseForceUpdate = false;
TimerHandle_t firebaseUpdateTimer;

void firebaseUpdateTimerCallback(TimerHandle_t firebaseUpdateTimer)
{
  FLAGfirebaseRegularUpdate = true;
}
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
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(devicename.c_str()); // define hostname
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[INIT] ERROR CONNECTING TO WIFI");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    vTaskDelay(3000);
  }
  FLAGwifiIsConnected = true;
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the API key (required) */
  config.api_key = API_KEY;
  /* Assign the RTDB URL */
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);

  Serial.print("Sign up new user... ");

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
  }
  else
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  FLAGfirebaseActive = true;

  firebaseUpdateTimer = xTimerCreate("firebaseUpdate", FIREBASE_UPDATE_INTERVAL_MS, pdTRUE, (void *)0, firebaseUpdateTimerCallback);
  xTimerStart(firebaseUpdateTimer, 0);
  while (1)
  {
    if (FLAGfirebaseForceUpdate || FLAGfirebaseRegularUpdate)
    {
      Serial.print("Firebase watermark: ");
      Serial.println(uxTaskGetStackHighWaterMark(NULL));
      Serial.println(ESP.getFreeHeap());
      FirebaseJson sensorinfoJson;
      for (uint8_t i = 0; i < sensorIndex; i++)
      {

        FirebaseJson deviceinfoJson;
        String device = "dev" + (String)i;
        deviceinfoJson.add("hw", sensorInfo[i].hw);
        deviceinfoJson.add("state", sensorInfo[i].state);
        deviceinfoJson.add("battery_voltage", sensorInfo[i].battery_voltage);
        deviceinfoJson.add("node_type", sensorInfo[i].node_type);
        deviceinfoJson.add("central_id", sensorInfo[i].central_id);
        deviceinfoJson.add("self_id", sensorInfo[i].self_id);
        deviceinfoJson.add("endpoint", sensorInfo[i].endpoint);
        deviceinfoJson.add("trigd", sensorInfo[i].trigd);
        deviceinfoJson.add("lqi", sensorInfo[i].lqi);
        deviceinfoJson.add("rssi", sensorInfo[i].rssi);
        deviceinfoJson.add("alive", sensorInfoExt[i].alive);
        sensorinfoJson.set(device, deviceinfoJson);
      }
      sensorinfoJson.add("HEAD", "test");
      if (Firebase.updateNode(fbdo, "/", sensorinfoJson))
      {
      }
      else
      {
      }
      FLAGfirebaseForceUpdate = false;
      FLAGfirebaseRegularUpdate = false;
      vTaskDelay(250);
    }
    vTaskDelay(5);
  }
}

void managerTask(void *pvParameters)
{
  Serial1.write(ipc_get_list, sizeof(ipc_get_list));
  ipcDeviceUpdateTimer = xTimerCreate("ipcUpdate", 5000, pdTRUE, (void *)0, ipcDeviceUpdateCallback);
  xTimerStart(ipcDeviceUpdateTimer, 0);

  while (1)
  {
    if (uxQueueMessagesWaiting(ipc2ManagerDeviceInfoQueue) > 0)
    {
      Serial.print("manager watermark: ");
      Serial.println(uxTaskGetStackHighWaterMark(NULL));
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
        sensorInfoExt[tmpInfo.id].ipcResponsePending = tmpInfo.ipcResponsePending;
        if (uxQueueSpacesAvailable(manager2GuiDeviceIndexQueue) == 0)
        {
          xQueueReset(manager2GuiDeviceIndexQueue);
        }
        xQueueSend(manager2GuiDeviceIndexQueue, (void *)&tmpInfo.id, 0);
        if (sensorInfo[tmpInfo.id].state == S_ALERTING)
          FLAGfirebaseForceUpdate = true;
      }
    }
    else
    {
      vTaskDelay(1);
    }
  }
}