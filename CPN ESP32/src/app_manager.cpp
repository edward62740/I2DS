#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_pr.h"
#include "app_manager.h"
#include "app_common.h"
#include <FirebaseESP32.h>
#include "WiFi.h"
#include <addons/TokenHelper.h>
#include <esp_task_wdt.h>
#include <EEPROM.h>

// Struct to keep all device info passed over IPC
DeviceInfo selfInfo, sensorInfo[MAX_I2DS_DEVICE_COUNT];

// Sensor count high watermark
uint8_t sensorIndex = 0;

// Struct for CPN device tracking
DeviceInfoExt selfInfoExt, sensorInfoExt[MAX_I2DS_DEVICE_COUNT];

// Queue to pass updated info from ipc to manager to update structs
QueueHandle_t ipc2ManagerDeviceInfoQueue;

// Queue to hold all pending firebase requests before ipc
QueueHandle_t app2ManagerDeviceReqQueue;

String devicename = "I2DS CONTROL PANEL";
// Define the Firebase Data object
FirebaseData fbdo;
// Define the FirebaseAuth data for authentication data
FirebaseAuth auth;
// Define the FirebaseConfig data for config data
FirebaseConfig config;
// Assign the project host and api key (required)

// inform GUI to flash on alert
bool guiFlashOnAlert = false;

bool FLAGfirebaseRegularUpdate = true;
TimerHandle_t firebaseUpdateTimer;

TimerHandle_t firebaseConnectionTimer;

bool FLAGfirebaseForceUpdate = false;
uint8_t firebaseForceUpdateDeviceId = 0;

time_t guiCurrentTime = 0;

ErrCount err_count;

uint8_t last_reset_reason = 0x00;

/*! firebaseConnectionTimerCallback()
   @brief callback for no connection timer

   @note restarts the processor if connection cannot be re-established within FIREBASE_WIFI_MAX_NO_CONNECTION_MS

   @param firebaseConnectionTimer FreeRTOS timer handle
*/
void firebaseConnectionTimerCallback(TimerHandle_t firebaseConnectionTimer)
{
  EEPROM.write(0, RST_CONN);
  EEPROM.commit();
  ESP.restart();
}

/*! firebaseUpdateTimerCallback()
   @brief callback for Firebase regular updates

   @note sets flag at interval FIREBASE_UPDATE_INTERVAL_MS, aborted if request in progress

   @param firebaseUpdateTimer FreeRTOS timer handle
*/
void firebaseUpdateTimerCallback(TimerHandle_t firebaseUpdateTimer)
{
  FLAGfirebaseRegularUpdate = true;
}

/*! managerDeviceTimerCallback()
   @brief callback for Manager alive tracker

   @note clears alive flag if no msg from device within MANAGER_MAX_DEVICE_NOMSG_MS

   @param managerDeviceTimer FreeRTOS timer handle
*/
void managerDeviceTimerCallback(TimerHandle_t managerDeviceTimer)
{
  uint8_t id = (uint32_t)pvTimerGetTimerID(managerDeviceTimer);
  sensorInfoExt[id].alive = false;
  if (uxQueueSpacesAvailable(manager2GuiDeviceIndexQueue) == 0)
  {
    xQueueReset(manager2GuiDeviceIndexQueue);
  }
  xQueueSend(manager2GuiDeviceIndexQueue, (void *)&id, 0);
}

/*! firebaseErrorQueueCallback()
   @brief callback on firebase error

   @note increments FIREBASE_ERR_QUEUE_OVERFLOW counter if queue is full

   @param errorQueue Firebase QueueInfo class
*/
void firebaseErrorQueueCallback(QueueInfo errorQueue)
{
  if (errorQueue.isQueueFull())
  {
    err_count.FIREBASE_ERR_QUEUE_OVERFLOW++;
  }
}

/*! firebaseTask()
   @brief updates Firebase RTDB reguarly and when forced to by manager,
   listens to requests from firebase and passes them to IPC

   @note

   @param void
*/
void firebaseTask(void *pvParameters)
{
  esp_task_wdt_init(15, true);
  EEPROM.begin(1); // Reset reason byte
  last_reset_reason = EEPROM.read(0);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(devicename.c_str()); // define hostname
  while (WiFi.status() != WL_CONNECTED)
  {
    APP_LOG_INFO("[INIT] ERROR CONNECTING TO WIFI");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    vTaskDelay(FIREBASE_WIFI_RETRY_INTERVAL_MS);
  }
  FLAGwifiIsConnected = true;

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Firebase configs
  Firebase.reconnectWiFi(true);
  Firebase.setMaxRetry(fbdo, MAX_FIREBASE_SET_RETRY);
  Firebase.setMaxErrorQueue(fbdo, MAX_FIREBASE_ERROR_QUEUE);
  Firebase.beginAutoRunErrorQueue(fbdo, firebaseErrorQueueCallback);
  fbdo.setResponseSize(FIREBASE_DATA_OBJECT_PAYLOAD_SIZE_BYTES);
  config.timeout.wifiReconnect = 10 * 1000;
  config.timeout.socketConnection = 6 * 1000;
  config.timeout.serverResponse = 8 * 1000;
  config.timeout.rtdbKeepAlive = 30 * 1000;
  config.timeout.rtdbStreamReconnect = 1 * 1000;
  config.timeout.rtdbStreamError = 3 * 1000;
  Firebase.signUp(&config, &auth, "", "");
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  FLAGfirebaseActive = true;
  firebaseUpdateTimer = xTimerCreate("firebaseUpdate", FIREBASE_UPDATE_INTERVAL_MS, pdTRUE, (void *)0, firebaseUpdateTimerCallback);
  firebaseConnectionTimer = xTimerCreate("firebaseConnection", FIREBASE_WIFI_MAX_NO_CONNECTION_MS, pdTRUE, (void *)0, firebaseConnectionTimerCallback);
  xTimerStart(firebaseUpdateTimer, 0);
  if (!Firebase.beginStream(fbdo, "/req/send"))
  {
    APP_LOG_INFO(fbdo.errorReason());
  }
  /* ERRATA => FIREBASE DOES NOT RETURN AS ARRAYS UNLESS MORE THAN HALF OF THE MAX VALUES ARE NON-NULL
     WORKAROUND => MAX DEVICE COUNT IS MAX_I2DS_DEVICE_COUNT; FILL UNUSED ARRAYS MAX_I2DS_DEVICE_COUNT + 1 to MAX_I2DS_DEVICE_COUNT * 3 WITH VALUES
  */
  FirebaseJson errataJson;
  for (uint8_t i = MAX_I2DS_DEVICE_COUNT + 1; i < MAX_I2DS_DEVICE_COUNT * 3; i++)
  {
    errataJson.add((String)i, 7);
  }

  Firebase.updateNode(fbdo, "/req/send", errataJson);
  // start timestamp
  FirebaseJson initTsJson;
  initTsJson.set("timestamp/.sv", "timestamp");
  Firebase.updateNode(fbdo, "/boot", initTsJson);

  while (1)
  {

    if (Firebase.ready() && FLAGfirebaseForceUpdate)
    {
      FirebaseJson deviceinfoJson;
      String device = "/devices/dev" + (String)firebaseForceUpdateDeviceId;
      deviceinfoJson.add("state", sensorInfo[firebaseForceUpdateDeviceId].state);
      deviceinfoJson.add("trigd", sensorInfo[firebaseForceUpdateDeviceId].trigd);
      if (!Firebase.updateNode(fbdo, device, deviceinfoJson))
      {
        err_count.FIREBASE_FORCED_UPDATE_FAIL++;
      }
      FLAGfirebaseForceUpdate = false;
    }

    else if (fbdo.streamAvailable() && !FLAGipcResponsePending)
    {
      if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_array)
      {
        bool tmpAccessed = false;
        uint8_t tmpClearList[MAX_FIREBASE_REQUEST_QUEUE];
        uint8_t tmpClearIndex = 0;
        FirebaseJsonData result;
        FirebaseJsonArray *arr = fbdo.to<FirebaseJsonArray *>();
        for (uint8_t i = 0; i < sensorIndex; i++)
        {
          arr->get(result, i);
          if (result.to<int>() != 0 && i >= 0 && i <= sensorIndex && (result.to<int>() == (int)S_ACTIVE || result.to<int>() == (int)S_INACTIVE))
          {
            FirebaseReq_t tmpReq;
            tmpReq.id = sensorInfo[i].self_id;
            tmpReq.state = (uint8_t)result.to<int>();
            tmpReq.index = i;
            tmpClearList[tmpClearIndex] = i;
            tmpClearIndex++;
            if (uxQueueSpacesAvailable(app2ManagerDeviceReqQueue) == 0)
            {
              xQueueReset(app2ManagerDeviceReqQueue);
              err_count.MANAGER_QUEUE_REQ_OVERFLOW++;
              APP_LOG_INFO("NO SPACE QUEUE");
            }
            if (xQueueSend(app2ManagerDeviceReqQueue, (void *)&tmpReq, 0) == 0)
            {
              err_count.MANAGER_QUEUE_REQ_FAIL++;
              APP_LOG_INFO("ERROR QUEUE");
            }
            tmpAccessed = true;
          }
        }
        /* ERRATA => DELETING NODE ENDS THE STREAM
           WORKAROUND => DELETE ONLY AFTER ALL ARRAY VALUES ARE ADDED TO THE QUEUE
        */
        if (tmpAccessed && tmpClearIndex != 0)
          for (uint8_t i = 0; i < tmpClearIndex; i++)
          {
            String node = "/req/send/" + (String)tmpClearList[i];
            Firebase.deleteNode(fbdo, node);
          }
      }
    }

    else if (Firebase.ready() && FLAGfirebaseRegularUpdate && !FLAGfirebaseForceUpdate)
    {
      FLAGwifiIsConnected = true;
      if (!Firebase.authenticated())
        err_count.FIREBASE_AUTH_ERR++;
      guiCurrentTime = Firebase.getCurrentTime();
      Firebase.getBool(fbdo, "/req/warn");
      guiFlashOnAlert = (bool)fbdo.to<bool>();
      FirebaseJson sensorinfoJson;
      APP_LOG_INFO("REGULAR UPDATE RUNNING");
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
      errJson.add("IPC_TOTAL_BYTES_EXCHANGED", err_count.IPC_TOTAL_BYTES_EXCHANGED);
      errJson.add("IPC_REQUEST_SEND_NOACK", err_count.IPC_REQUEST_SEND_NOACK);
      errJson.add("IPC_REQUEST_SEND_FAIL", err_count.IPC_REQUEST_SEND_FAIL);
      errJson.add("IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW", err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW);
      errJson.add("IPC_QUEUE_SEND_DEVICEINFO_FAIL", err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL);
      errJson.add("IPC_CHANGE_INVALID", err_count.IPC_CHANGE_INVALID);
      errJson.add("IPC_CHANGE_INDEX_OUT_OF_BOUNDS", err_count.IPC_CHANGE_INDEX_OUT_OF_BOUNDS);
      errJson.add("MANAGER_QUEUE_SEND_DEVICEINDEX_OVERFLOW", err_count.MANAGER_QUEUE_SEND_DEVICEINDEX_OVERFLOW);
      errJson.add("MANAGER_QUEUE_SEND_DEVICEINDEX_FAIL", err_count.MANAGER_QUEUE_SEND_DEVICEINDEX_FAIL);
      errJson.add("MANAGER_QUEUE_RECEIVE_OUT_OF_BOUNDS", err_count.MANAGER_QUEUE_RECEIVE_OUT_OF_BOUNDS);
      errJson.add("MANAGER_QUEUE_REQ_OVERFLOW", err_count.MANAGER_QUEUE_REQ_OVERFLOW);
      errJson.add("MANAGER_QUEUE_REQ_FAIL", err_count.MANAGER_QUEUE_REQ_FAIL);
      errJson.add("FIREBASE_AUTH_ERR", err_count.FIREBASE_AUTH_ERR);
      errJson.add("FIREBASE_REGULAR_UPDATE_FAIL", err_count.FIREBASE_REGULAR_UPDATE_FAIL);
      errJson.add("FIREBASE_FORCED_UPDATE_FAIL", err_count.FIREBASE_FORCED_UPDATE_FAIL);
      errJson.add("FIREBASE_ERR_QUEUE_OVERFLOW", err_count.FIREBASE_ERR_QUEUE_OVERFLOW);
      errJson.add("FIREBASE_NETWORK_FAIL", err_count.FIREBASE_NETWORK_FAIL);
      errJson.add("IPC_TOTAL_EXCHANGES", err_count.IPC_TOTAL_EXCHANGES);
      errJson.add("FREE HEAP SIZE", xPortGetFreeHeapSize());
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
      prPowerDc ? iJson.add("pr", "true") : iJson.add("pr", "false");
      iJson.add("rst", last_reset_reason);

      sensorinfoJson.set("/info", iJson);

      if (!Firebase.updateNode(fbdo, "/", sensorinfoJson))
      {
        err_count.FIREBASE_REGULAR_UPDATE_FAIL++;
        xTimerStart(firebaseConnectionTimer, 0);
        FLAGwifiIsConnected = false;
      }
      else
      {
        xTimerStop(firebaseConnectionTimer, 0);
        xTimerReset(firebaseConnectionTimer, 0);
      }

      APP_LOG_INFO("REGULAR UPDATE DONE");
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

/*! managerTask()
   @brief manages and tracks device info, alive etc. Receives updates from IPC via ipc2ManagerDeviceInfoQueue,
   informs GUI of pending updates via manager2GuiDeviceIndexQueue and forces Firebase to update on S_ALERTING on any device.

   @note

   @param void
*/
void managerTask(void *pvParameters)
{
  delay(100);
  Serial1.write(ipc_get_list, sizeof(ipc_get_list));
  while (1)
  {
    if ((uxQueueMessagesWaiting(app2ManagerDeviceReqQueue) > 0) && !FLAGipcResponsePending)
    {
      FirebaseReq_t tmpReq;
      if (xQueueReceive(app2ManagerDeviceReqQueue, (void *)&tmpReq, 1) == pdPASS)
      {
        APP_LOG_INFO("REMOVED FROM QUEUE");
        ipcSender(tmpReq.id, tmpReq.state);
        guiDeviceSelectedIndex = tmpReq.index;
        guiDeviceSelectedSw = true;
      }
    }
    if (uxQueueMessagesWaiting(ipc2ManagerDeviceInfoQueue) > 0)
    {
      APP_LOG_INFO(ESP.getFreeHeap());
      DeviceInfoExt tmpInfo;
      if (xQueueReceive(ipc2ManagerDeviceInfoQueue, (void *)&tmpInfo, 1) == pdPASS)
      {
        /* PARSE INCOMING CHANGES BASED ON VALUE RETURNED BY cmpDeviceInfo() */
        uint16_t index = tmpInfo.DeviceInfoChangeIndex;
        if (~index & 1)
          sensorInfo[tmpInfo.id].hw = tmpInfo.info.hw;
        if (~index & 2)
        {
          if ((sensorInfo[tmpInfo.id].state != tmpInfo.info.state) && (tmpInfo.info.state == S_ACTIVE || tmpInfo.info.state == S_ALERTING ||
                                                                       tmpInfo.info.state == S_INACTIVE || tmpInfo.info.state == S_FAULT_HW || tmpInfo.info.state == S_COLDSTART || tmpInfo.info.state == S_FAULT_OPN || tmpInfo.info.state == S_WATCHING))
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
        // Informs GUI to update based on device INDEX
        if (xQueueSend(manager2GuiDeviceIndexQueue, (void *)&tmpInfo.id, 0) == 0)
        {
          err_count.MANAGER_QUEUE_SEND_DEVICEINDEX_FAIL++;
        }

        // If alerting, force firebase to update
        if (sensorInfo[tmpInfo.id].state == S_ALERTING)
          FLAGfirebaseForceUpdate = true;
        firebaseForceUpdateDeviceId = tmpInfo.id;
      }
    }

    vTaskDelay(10);
  }
}