#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include "app_common.h"

// Timer for IPC response timeout
TimerHandle_t ipcDeviceResponseTimer;
// Timer array for device alive tracking
TimerHandle_t managerDeviceTimer[MAX_I2DS_DEVICE_COUNT];

// IPC consts
uint8_t IPC_START = 0xAF;
uint8_t IPC_END = 0xAC;

// Fixed tx msg array
uint8_t ipc_get_list[3] = {IPC_START, (uint8_t)IPC_LIST_CTS, IPC_END};
uint8_t ipc_set_sensor_state[6] = {IPC_START, (uint8_t)IPC_REQUEST, 0x00, 0x00, 0x00, IPC_END};

// IPC rx buffer
char ipc_recv_buffer[255];

// Flags to keep track of ipc status
bool FLAGguiUpdatePending = false;
bool FLAGipcInternalAckPending = false;

// Variables for ipc timeout handling
uint16_t tmpTimerId = 0;
uint8_t tmpTimerState = 0;
uint8_t tmpResendCount = 0;
uint8_t tmpRetryCount = 0;

/*! ipcRespTimerCallback()
   @brief callback for IPC timer timeout

   @note called if no IPC_REQUEST_DONE within IPC_RESPONSE_TIMEOUT_MS

   @param ipcDeviceResponseTimer FreeRTOS timer handle
*/
void ipcRespTimerCallback(TimerHandle_t ipcDeviceResponseTimer)
{
    if (FLAGipcInternalAckPending)
        err_count.IPC_REQUEST_SEND_NOACK++;

    err_count.IPC_REQUEST_SEND_FAIL++;
    sensorInfoExt[tmpTimerId].ipcResponsePending = false;
    xTimerStop(ipcDeviceResponseTimer, 0);
    if (tmpResendCount < MAX_IPC_REQUEST_RETRY)
    {
        tmpResendCount++;
        ipcSender(tmpTimerId, tmpTimerState);
    }
    else
        tmpResendCount = 0;
}

/*! ipcTask()
   @brief task to handle ipc communication with efr32fg23

   @note

   @param void
*/
void ipcTask(void *pvParameters)
{
    ipcDeviceResponseTimer = xTimerCreate("ipcResp", IPC_RESPONSE_TIMEOUT_MS, pdTRUE, (void *)0, ipcRespTimerCallback);
    delay(100);
    pinMode(ACT_LED, OUTPUT);
    while (1)
    {
        if (Serial1.available()) // receive bytes if avail and parse
        {
            digitalWrite(ACT_LED, HIGH);
            size_t ret = 0;
            ret = Serial1.readBytesUntil(IPC_END, (char *)ipc_recv_buffer, sizeof(ipc_recv_buffer)); // read until end byte
            ipcParser(ipc_recv_buffer, ret);
            digitalWrite(ACT_LED, LOW);
        }
        vTaskDelay(10);
    }
}

/*! ipcSender()
   @brief function called by gui/firebase task to send commands to devices

   @note returns false if awaiting another request, at most MAX_IPC_REQUEST_RETRY * IPC_RESPONSE_TIMEOUT_MS from previous call

   @param id device ID to change *NOT uint8_t INDEX*
   @param state requested state
   @return bool success
*/
bool ipcSender(uint16_t id, uint8_t state)
{
    sensorInfoExt[id].ipcResponsePending = true;
    ipc_set_sensor_state[2] = 0xFF & (uint8_t)id >> 8;
    ipc_set_sensor_state[3] = 0xFF & (uint8_t)id;
    ipc_set_sensor_state[4] = 0xFF & (uint8_t)state;
    err_count.IPC_TOTAL_BYTES_EXCHANGED += static_cast<uint32_t>(sizeof(ipc_set_sensor_state));
    Serial1.write(ipc_set_sensor_state, sizeof(ipc_set_sensor_state));
    FLAGipcInternalAckPending = true;
    tmpTimerId = id;
    tmpTimerState = state;
    xTimerStart(ipcDeviceResponseTimer, 0);
    return true;
}

/*! ipcParser()
   @brief parses incoming IPC messages and acts appropriately

   @note

   @param buffer pointer to rx serial buffer
   @param len size of rxed buffer
   @return bool success
*/
bool ipcParser(char *buffer, size_t len)
{
    err_count.IPC_TOTAL_EXCHANGES++;
    err_count.IPC_TOTAL_BYTES_EXCHANGED += (uint32_t)len;
    char tmpBuf[len];
    memcpy(tmpBuf, buffer, len * sizeof(char));
    APP_LOG_INFO(len);
    APP_LOG_INFO((int)tmpBuf[1]);
    APP_LOG_INFO((int)tmpBuf[2]);
    APP_LOG_INFO((int)tmpBuf[3]);
    if (tmpBuf[0] != (char)IPC_START) // dump message if incorrect start byte
        return false;
    if (tmpBuf[1] != (len -1)) // dump message if incorrect length
        return false;

    switch (tmpBuf[2]) // determine message type
    {
    // Response to IPC_LIST_CTS, done at startup
    case IPC_LIST:
    {
        if (((len - 5) % 15) != 0)
        {
            Serial1.write(ipc_get_list, sizeof(ipc_get_list));
            break;
        }
        for (uint8_t i = 0; i < (uint8_t)(tmpBuf[3]); i++)
        {
            uint8_t tmpIndex = 0;
            DeviceInfo tmpInfo;

            tmpInfo.hw = (uint8_t)tmpBuf[4 + (i * 15)];
            tmpInfo.state = (uint8_t)tmpBuf[5 + (i * 15)];
            tmpInfo.battery_voltage = (uint32_t)((tmpBuf[6 + (i * 15)] << 24) | (tmpBuf[7 + (i * 15)] << 16) | (tmpBuf[8 + (i * 15)] << 8) | tmpBuf[9 + (i * 15)]);
            tmpInfo.node_type = (uint8_t)tmpBuf[10 + (i * 15)];
            tmpInfo.central_id = (uint16_t)((tmpBuf[11 + (i * 15)] << 8) | tmpBuf[12 + (i * 15)]);
            tmpInfo.self_id = (uint16_t)((tmpBuf[13 + (i * 15)] << 8) | tmpBuf[14 + (i * 15)]);
            tmpInfo.endpoint = (uint8_t)tmpBuf[15 + (i * 15)];
            tmpInfo.trigd = (uint8_t)tmpBuf[16 + (i * 15)];
            tmpInfo.rssi = (int8_t)tmpBuf[17 + (i * 15)];
            tmpInfo.lqi = (uint8_t)tmpBuf[18 + (i * 15)];
            Serial.println("DETECTED DEVICE WITH ID " + String(tmpInfo.self_id));
            for (uint8_t i = 0; i < sensorIndex; i++)
            {
                if (sensorInfo[i].self_id == tmpInfo.self_id)
                {
                    DeviceInfoExt queueSend;

                    queueSend.DeviceInfoChangeIndex = cmpDeviceInfo(&tmpInfo, &sensorInfo[i]);
                    if (queueSend.DeviceInfoChangeIndex != 1023)
                    {
                        queueSend.info = tmpInfo;
                        queueSend.id = i;
                        queueSend.guiUpdatePending = true;

                        if (uxQueueSpacesAvailable(ipc2ManagerDeviceInfoQueue) == 0)
                        {
                            xQueueReset(ipc2ManagerDeviceInfoQueue);
                            err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW++;
                        }

                        if (xQueueSend(ipc2ManagerDeviceInfoQueue, (void *)&queueSend, 0) == 0)
                            err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL++;
                    }
                    break;
                }
                else
                    tmpIndex++;
            }
            if (tmpIndex == sensorIndex && (tmpInfo.self_id < 255) && (tmpInfo.self_id != 0))
            {
                DeviceInfoExt queueSend;
                queueSend.DeviceInfoChangeIndex = 0;
                queueSend.info = tmpInfo;
                queueSend.id = i;
                queueSend.alive = true;
                managerDeviceTimer[i] = xTimerCreate("managerDevice", MANAGER_MAX_DEVICE_NOMSG_MS, pdTRUE, (void *)((uint32_t)i), managerDeviceTimerCallback);
                xTimerStart(managerDeviceTimer[i], 0);
                Serial.println("NEW DEVICE ADDED WITH ID: " + String(tmpInfo.self_id));
                queueSend.guiUpdatePending = true;
                if (uxQueueSpacesAvailable(ipc2ManagerDeviceInfoQueue) == 0)
                {
                    xQueueReset(ipc2ManagerDeviceInfoQueue);
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW++;
                }

                if (xQueueSend(ipc2ManagerDeviceInfoQueue, (void *)&queueSend, 0) == 0)
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL++;
                sensorIndex++;
            }
            sensorIndex = tmpBuf[3];
        }

        break;
    }
    // regular report fmt
    case IPC_REPORT:
    {
        if (len != 13)
            break;
        Serial.print("REPORT");
        DeviceInfo tmpInfo;
        tmpInfo.self_id = (uint16_t)((tmpBuf[3] << 8) | tmpBuf[4]);
        tmpInfo.battery_voltage = (uint32_t)((tmpBuf[5] << 24) | (tmpBuf[6] << 16) | (tmpBuf[7] << 8) | tmpBuf[8]);
        tmpInfo.state = (uint8_t)tmpBuf[9];
        tmpInfo.rssi = (int8_t)tmpBuf[10];
        tmpInfo.lqi = (uint8_t)tmpBuf[11];
        for (uint8_t i = 0; i < sensorIndex; i++)
        {
            if (sensorInfo[i].self_id == tmpInfo.self_id)
            {
                DeviceInfoExt queueSend;
                APP_LOG_INFO(tmpInfo.state);
                queueSend.info.battery_voltage = tmpInfo.battery_voltage;
                queueSend.info.state = tmpInfo.state;
                queueSend.info.rssi = tmpInfo.rssi;
                queueSend.info.lqi = tmpInfo.lqi;
                queueSend.id = i;
                queueSend.alive = true;
                queueSend.DeviceInfoChangeIndex = 249;
                if (tmpInfo.state != (uint8_t)S_ALERTING)
                {
                    queueSend.info.trigd = 0;
                    queueSend.DeviceInfoChangeIndex = 121;
                }

                queueSend.guiUpdatePending = true;
                xTimerReset(managerDeviceTimer[i], 0);
                if (uxQueueSpacesAvailable(ipc2ManagerDeviceInfoQueue) == 0)
                {
                    xQueueReset(ipc2ManagerDeviceInfoQueue);
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW++;
                }

                if (xQueueSend(ipc2ManagerDeviceInfoQueue, (void *)&queueSend, 0) == 0)
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL++;
                break;
            }
        }

        break;
    }

    // message upon change of device status
    case IPC_CHANGE:
    {
        if (len != 9)
            break;
        uint16_t id = (tmpBuf[3] << 8) | tmpBuf[4];
        uint8_t state = tmpBuf[5];
        uint8_t count = tmpBuf[7];
        uint8_t tmpIndex = 0;
        for (uint8_t i = 0; i < sensorIndex; i++)
        {
            if (sensorInfo[i].self_id == id)
            {
                DeviceInfoExt queueSend;
                queueSend.DeviceInfoChangeIndex = 893;
                if (tmpBuf[6] == 0x00) // started
                    state = S_ALERTING;
                if (tmpBuf[6] == 0x01) // ended
                    state = S_ACTIVE;
                queueSend.info.state = state;
                queueSend.info.trigd = count;
                queueSend.id = i;
                queueSend.guiUpdatePending = true;
                if (uxQueueSpacesAvailable(ipc2ManagerDeviceInfoQueue) == 0)
                {
                    xQueueReset(ipc2ManagerDeviceInfoQueue);
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW++;
                }

                if (xQueueSend(ipc2ManagerDeviceInfoQueue, (void *)&queueSend, 0) == 0)
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL++;
                break;
            }
            else
                tmpIndex++;
        }
        if (tmpIndex == sensorIndex)
        {
            if ((uint8_t)tmpBuf[6] == 2)
            {
                err_count.IPC_CHANGE_INDEX_OUT_OF_BOUNDS++;
                Serial1.write(ipc_get_list, sizeof(ipc_get_list));
            }
            else
            {
                err_count.IPC_CHANGE_INVALID++;
            }
        }

        break;
    }
    // response upon completion of IPC_REQUEST
    case IPC_REQUEST_DONE:
    {
        if (len != 8)
            break;

        uint16_t id = ((tmpBuf[3] << 8) | tmpBuf[4]);
        for (uint8_t i = 0; i < sensorIndex; i++)
        {
            if (sensorInfo[i].self_id == id)
            {
                if ((tmpBuf[6] == 0x00) && MAX_IPC_REQUEST_RESEND)
                {
                    tmpRetryCount++;
                }

                tmpRetryCount = 0;
                tmpResendCount = 0;
                xTimerStop(ipcDeviceResponseTimer, 0);
                DeviceInfoExt queueSend;
                queueSend.DeviceInfoChangeIndex = 1021;
                queueSend.info.state = tmpBuf[5];
                queueSend.id = i;
                queueSend.guiUpdatePending = true;
                queueSend.ipcResponsePending = false;
                queueSend.alive = true;
                if (uxQueueSpacesAvailable(ipc2ManagerDeviceInfoQueue) == 0)
                {
                    xQueueReset(ipc2ManagerDeviceInfoQueue);
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW++;
                }

                if (xQueueSend(ipc2ManagerDeviceInfoQueue, (void *)&queueSend, 0) == 0)
                    err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL++;

                break;
            }
        }

        break;
    }
    // response upon rx of IPC_REQUEST
    case IPC_REQUEST_ACK:
    {
        if (len != 4)
            break;
            FLAGipcInternalAckPending = false;
        break;
    }
    }
    return true;
}
/*! cmpDeviceInfo()
   @brief compares the two param and returns the difference

   @note
   @param s1 pointer to DeviceInfo struct
   @param s2 pointer to DeviceInfo struct
   @return uint16_t to indicate binary pos of changed items in struct
*/
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