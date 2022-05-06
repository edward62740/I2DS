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
uint8_t firebaseForceUpdateDeviceId = 0;
TimerHandle_t firebaseUpdateTimer;
ErrCount err_count = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void firebaseUpdateTimerCallback(TimerHandle_t firebaseUpdateTimer)
{
  FLAGfirebaseRegularUpdate = true;
}
void managerDeviceTimerCallback(TimerHandle_t managerDeviceTimer)
{
  uint8_t id = (uint32_t)pvTimerGetTimerID(managerDeviceTimer);
  sensorInfoExt[id].alive = false;
  APP_LOG_INFO(id);
  if (uxQueueSpacesAvailable(manager2GuiDeviceIndexQueue) == 0)
  {
    xQueueReset(manager2GuiDeviceIndexQueue);
  }
  xQueueSend(manager2GuiDeviceIndexQueue, (void *)&id, 0);
}
void ipcDeviceUpdateCallback(TimerHandle_t ipcDeviceUpdateTimer)
{
  updateDevice = true;
  APP_LOG_INFO("Updated");
}

void firebaseErrorQueueCallback(QueueInfo errorQueue)
{

  if (errorQueue.isQueueFull())
  {
    APP_LOG_INFO("Queue is full");
  }

  APP_LOG_INFO("Remaining queues: ");
  APP_LOG_INFO(errorQueue.totalQueues());

  APP_LOG_INFO("Being processed queue ID: ");
  APP_LOG_INFO(errorQueue.currentQueueID());

  APP_LOG_INFO("Data type:");
  APP_LOG_INFO(errorQueue.dataType());

  APP_LOG_INFO("Method: ");
  APP_LOG_INFO(errorQueue.firebaseMethod());

  APP_LOG_INFO("Path: ");
  APP_LOG_INFO(errorQueue.dataType());

  APP_LOG_INFO();
}
void firebaseTask(void *pvParameters)
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(devicename.c_str()); // define hostname
  while (WiFi.status() != WL_CONNECTED)
  {
    APP_LOG_INFO("[INIT] ERROR CONNECTING TO WIFI");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    vTaskDelay(3000);
  }
  FLAGwifiIsConnected = true;
  /* Assign the API key (required) */
  config.api_key = API_KEY;
  /* Assign the RTDB URL */
  config.database_url = DATABASE_URL;
  // Optional, set AP reconnection in setup()
  Firebase.reconnectWiFi(true);
  Firebase.setMaxRetry(fbdo, 3);
  Firebase.setMaxErrorQueue(fbdo, 10);
  Firebase.beginAutoRunErrorQueue(fbdo, firebaseErrorQueueCallback);
  fbdo.setResponseSize(8192);
  // WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;
  // Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;
  // Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;
  // RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;
  // RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;
  // RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  // will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;
  APP_LOG_INFO("Sign up new user... ");

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    APP_LOG_INFO("ok");
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  FLAGfirebaseActive = true;
  firebaseUpdateTimer = xTimerCreate("firebaseUpdate", FIREBASE_UPDATE_INTERVAL_MS, pdTRUE, (void *)0, firebaseUpdateTimerCallback);
  xTimerStart(firebaseUpdateTimer, 0);
  // Firebase.deleteNode(fbdo, "/");
  if (!Firebase.beginStream(fbdo, "/req/send"))
  {
    APP_LOG_INFO(fbdo.errorReason());
  }
  FirebaseJson errataJson;
  for (uint8_t i = 30 + 1; i < 30 * 3; i++)
  {
    errataJson.add((String)i, 7);
  }
  Firebase.updateNode(fbdo, "/req/send", errataJson);
  while (1)
  {
  
    if (!Firebase.authenticated())
      err_count.FIREBASE_AUTH_ERR++;
    if (Firebase.ready() && FLAGfirebaseForceUpdate)
    {
      FirebaseJson deviceinfoJson;
      String device = "/devices/dev" + (String)firebaseForceUpdateDeviceId;
      deviceinfoJson.add("state", sensorInfo[firebaseForceUpdateDeviceId].state);
      deviceinfoJson.add("trigd", sensorInfo[firebaseForceUpdateDeviceId].trigd);
      if (Firebase.updateNode(fbdo, device, deviceinfoJson))
      {
      }
      else
      {
      }
      FLAGfirebaseForceUpdate = false;
    }
    else if (fbdo.streamAvailable() && !FLAGipcResponsePending)
    {

      if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_array)
      {
        FirebaseJsonData result;
        FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();
        for (size_t i = 0; i < sensorIndex; i++)
        {
          arr->get(result, i);
          if (result.to<int>() != 0 && i < sensorIndex)
          {
            if (i >= 0 && i <= sensorIndex && (result.to<int>() == (int)S_ACTIVE || result.to<int>() == (int)S_INACTIVE))
            {
              ipcSender(sensorInfo[i].self_id, (uint8_t)result.to<int>());
              selection = i;
              swflag = true;
            }
            String node = "/req/send/" + (String)i;
            Firebase.deleteNode(fbdo, node);
            break;
          }
        }
      }
    }

    else if (Firebase.ready() && FLAGfirebaseRegularUpdate)
    {
      APP_LOG_INFO("free heap: ");
      APP_LOG_INFO(xPortGetFreeHeapSize());
      FirebaseJson sensorinfoJson;

      // Firebase.setTimestamp(fbdo, "/info/timestamp");
      for (uint8_t i = 0; i < sensorIndex; i++)
      {

        FirebaseJson deviceinfoJson;
        String device = "/devices/dev" + (String)i;

        if (sensorInfoExt[i].alive)
        {
          deviceinfoJson.set("timestamp/.sv", "timestamp");
        }
        else
        {
          Firebase.getDouble(fbdo, device + "/timestamp");
          deviceinfoJson.add("timestamp", fbdo.to<uint64_t>());
        }
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
      FirebaseJson errJson;
      errJson.add("IPC_REQUEST_SEND_NOACK", err_count.IPC_REQUEST_SEND_NOACK);
      errJson.add("IPC_REQUEST_SEND_FAIL", err_count.IPC_REQUEST_SEND_FAIL);
      errJson.add("IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW", err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW);
      errJson.add("IPC_QUEUE_SEND_DEVICEINFO_FAIL", err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL);
      errJson.add("IPC_CHANGE_INVALID", err_count.IPC_CHANGE_INVALID);
      errJson.add("IPC_CHANGE_INDEX_OUT_OF_BOUNDS", err_count.IPC_CHANGE_INDEX_OUT_OF_BOUNDS);
      errJson.add("MANAGER_QUEUE_SEND_DEVICEINDEX_OVERFLOW", err_count.MANAGER_QUEUE_SEND_DEVICEINDEX_OVERFLOW);
      errJson.add("MANAGER_QUEUE_SEND_DEVICEINDEX_FAIL", err_count.MANAGER_QUEUE_SEND_DEVICEINDEX_FAIL);
      errJson.add("MANAGER_QUEUE_RECEIVE_OUT_OF_BOUNDS", err_count.MANAGER_QUEUE_RECEIVE_OUT_OF_BOUNDS);
      errJson.add("FIREBASE_AUTH_ERR", err_count.FIREBASE_AUTH_ERR);
      errJson.add("FIREBASE_REGULAR_UPDATE_FAIL", err_count.FIREBASE_REGULAR_UPDATE_FAIL);
      errJson.add("FIREBASE_FORCED_UPDATE_FAIL", err_count.FIREBASE_FORCED_UPDATE_FAIL);
      errJson.add("FIREBASE_ERR_QUEUE_OVERFLOW", err_count.FIREBASE_ERR_QUEUE_OVERFLOW);
      errJson.add("FIREBASE_NETWORK_FAIL", err_count.FIREBASE_NETWORK_FAIL);
      errJson.add("IPC_TOTAL_EXCHANGES", err_count.IPC_TOTAL_EXCHANGES);
      sensorinfoJson.set("/errors", errJson);
      FirebaseJson iJson;
      iJson.add("con", sensorIndex);
      uint8_t tmpcount = 0;
      for (uint8_t i = 0; i < sensorIndex; i++)
      {
        if (sensorInfoExt[i].alive)
          tmpcount++;
      }
      iJson.add("devs", tmpcount);
      iJson.add("wifi", WIFI_SSID);
      iJson.add("sec", "true");
      iJson.add("pr", "true");
      sensorinfoJson.set("/info", iJson);
      if (Firebase.updateNode(fbdo, "/", sensorinfoJson))
      {
      }
      else
      {
      }
      FLAGfirebaseRegularUpdate = false;
    }

    if (!Firebase.readStream(fbdo))
    {
      APP_LOG_INFO(fbdo.errorReason());
    }

    if (fbdo.streamTimeout())
    {
      APP_LOG_INFO("Stream timeout, resume streaming...");
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
      DeviceInfoExt tmpInfo;
      if (xQueueReceive(ipc2ManagerDeviceInfoQueue, (void *)&tmpInfo, 1) == pdPASS)
      {
        uint16_t index = tmpInfo.DeviceInfoChangeIndex;
        if (~index & 1)
          sensorInfo[tmpInfo.id].hw = tmpInfo.info.hw;
        if (~index & 2)
        {
          if ((sensorInfo[tmpInfo.id].state != tmpInfo.info.state) && (tmpInfo.info.state == S_ACTIVE || tmpInfo.info.state == S_ALERTING || tmpInfo.info.state == S_INACTIVE || tmpInfo.info.state == S_FAULT_HW || tmpInfo.info.state == S_COLDSTART))
          sensorInfo[tmpInfo.id].state = tmpInfo.info.state;
        }

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
          err_count.MANAGER_QUEUE_SEND_DEVICEINDEX_OVERFLOW++;
        }
        if (xQueueSend(manager2GuiDeviceIndexQueue, (void *)&tmpInfo.id, 0) == 0)
        {
          err_count.MANAGER_QUEUE_SEND_DEVICEINDEX_FAIL++;
        }
        if (sensorInfo[tmpInfo.id].state == S_ALERTING)
          FLAGfirebaseForceUpdate = true;
        firebaseForceUpdateDeviceId = tmpInfo.id;
      }
    }
    vTaskDelay(5);
  }
}