#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include "app_common.h"

uint8_t IPC_START = 0xAF;
uint8_t IPC_END = 0xAC;
char ipc_recv_buffer[255];
uint8_t ipc_get_list[3] = {IPC_START, (uint8_t)IPC_LIST_CTS, IPC_END};
uint8_t ipc_set_sensor_state[6] = {IPC_START, (uint8_t)IPC_REQUEST, 0x00, 0x00, 0x00, IPC_END};
bool FLAGguiUpdatePending = false;
bool FLAGipcResponsePending = false;
bool FLAGipcInternalAckPending = false;
uint16_t tmpTimerId = 0;
uint8_t tmpTimerState = 0;
uint8_t tmpResendCount = 0;
uint8_t tmpRetryCount = 0;
TimerHandle_t ipcDeviceResponseTimer;
TimerHandle_t managerDeviceTimer[30];

void ipcRespTimerCallback(TimerHandle_t ipcDeviceResponseTimer)
{

    if (FLAGipcInternalAckPending)
    {
        err_count.IPC_REQUEST_SEND_NOACK++;
    }
    err_count.IPC_REQUEST_SEND_FAIL++;
    sensorInfoExt[tmpTimerId].ipcResponsePending = false;
    FLAGipcResponsePending = false;
    xTimerStop(ipcDeviceResponseTimer, 0);
    if (tmpResendCount < MAX_IPC_REQUEST_RETRY)
    {
        tmpResendCount++;
        ipcSender(tmpTimerId, tmpTimerState);
    }
    else
    {
        tmpResendCount = 0;
    }
}

void ipcTask(void *pvParameters)
{
    ipcDeviceResponseTimer = xTimerCreate("ipcResp", MAX_IPC_RESPONSE_TIMEOUT_MS, pdTRUE, (void *)0, ipcRespTimerCallback);
    delay(100);
    pinMode(ACT_LED, OUTPUT);
    while (1)
    {
        if (Serial1.available())
        {
            digitalWrite(ACT_LED, HIGH);
            size_t ret = 0;
            ret = Serial1.readBytesUntil(IPC_END, (char *)ipc_recv_buffer, sizeof(ipc_recv_buffer));
            ipcParser(ipc_recv_buffer, ret);
            digitalWrite(ACT_LED, LOW);
        }
        if (updateDevice)
        {
            updateDevice = false;
        }

        vTaskDelay(5);
    }
}

void ipcSender(uint16_t id, uint8_t state)
{

    sensorInfoExt[id].ipcResponsePending = true;
    FLAGipcResponsePending = true;
    ipc_set_sensor_state[2] = 0xFF & (uint8_t)id >> 8;
    ipc_set_sensor_state[3] = 0xFF & (uint8_t)id;
    ipc_set_sensor_state[4] = 0xFF & (uint8_t)state;
    Serial1.write(ipc_set_sensor_state, sizeof(ipc_set_sensor_state));
    FLAGipcInternalAckPending = true;
    tmpTimerId = id;
    tmpTimerState = state;
    xTimerStart(ipcDeviceResponseTimer, 0);
}
bool ipcParser(char *buffer, size_t len)
{
    err_count.IPC_TOTAL_EXCHANGES++;
    if (len > 255)
        return false;
    char tmpBuf[len];
    memcpy(tmpBuf, buffer, len * sizeof(char));
    if (tmpBuf[0] != (char)IPC_START)
        return false;
    switch (tmpBuf[1])
    {
        // Response to IPC_LIST_CTS
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
            if (tmpIndex == sensorIndex)
            {
                DeviceInfoExt queueSend;
                queueSend.DeviceInfoChangeIndex = 0;
                queueSend.info = tmpInfo;
                queueSend.id = i;
                queueSend.alive = true;
                managerDeviceTimer[i] = xTimerCreate("managerDevice", MANAGER_MAX_DEVICE_NOMSG_MS, pdTRUE, (void *)((uint32_t)i), managerDeviceTimerCallback);
                xTimerStart(managerDeviceTimer[i], 0);
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
            sensorIndex = tmpBuf[2];
        }

        break;
    }
    case IPC_REPORT:
    {
        DeviceInfo tmpInfo;
        tmpInfo.self_id = (uint16_t)((tmpBuf[2] << 8) | tmpBuf[3]);
        tmpInfo.battery_voltage = (uint32_t)((tmpBuf[4] << 24) | (tmpBuf[5] << 16) | (tmpBuf[6] << 8) | tmpBuf[7]);
        tmpInfo.state = (uint8_t)tmpBuf[8];
        tmpInfo.rssi = (int8_t)tmpBuf[9];
        tmpInfo.lqi = (uint8_t)tmpBuf[10];
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
    // Message upon change of device status
    case IPC_CHANGE:
    {
        uint16_t id = (tmpBuf[2] << 8) | tmpBuf[3];
        uint8_t state = tmpBuf[4];
        uint8_t count = tmpBuf[6];
        uint8_t tmpIndex = 0;
        for (uint8_t i = 0; i < sensorIndex; i++)
        {
            if (sensorInfo[i].self_id == id)
            {
                DeviceInfoExt queueSend;
                queueSend.DeviceInfoChangeIndex = 893;
                if (tmpBuf[5] == 0x00) // started
                    state = S_ALERTING;
                if (tmpBuf[5] == 0x01) // ended
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
            if ((uint8_t)tmpBuf[5] == 2)
            {
                err_count.IPC_CHANGE_INDEX_OUT_OF_BOUNDS++;
                Serial1.write(ipc_get_list, sizeof(ipc_get_list));
            }
            else{
                err_count.IPC_CHANGE_INVALID++;
            }
        }

        break;
    }
    // Response upon completion of IPC_REQUEST
    case IPC_REQUEST_DONE:
    {
        if (FLAGipcResponsePending)
        {
            uint16_t id = ((tmpBuf[2] << 8) | tmpBuf[3]);
            for (uint8_t i = 0; i < sensorIndex; i++)
            {
                if (sensorInfo[i].self_id == id)
                {
                    if ((tmpBuf[5] == 0x00) && MAX_IPC_REQUEST_RESEND)
                    {
                        tmpRetryCount++;
                    }

                    tmpRetryCount = 0;
                    tmpResendCount = 0;
                    xTimerStop(ipcDeviceResponseTimer, 0);
                    DeviceInfoExt queueSend;
                    queueSend.DeviceInfoChangeIndex = 1021;
                    queueSend.info.state = tmpBuf[4];
                    queueSend.id = i;
                    queueSend.guiUpdatePending = true;
                    queueSend.ipcResponsePending = false;
                    if (uxQueueSpacesAvailable(ipc2ManagerDeviceInfoQueue) == 0)
                    {
                        xQueueReset(ipc2ManagerDeviceInfoQueue);
                        err_count.IPC_QUEUE_SEND_DEVICEINFO_OVERFLOW++;
                    }

                    if (xQueueSend(ipc2ManagerDeviceInfoQueue, (void *)&queueSend, 0) == 0)
                        err_count.IPC_QUEUE_SEND_DEVICEINFO_FAIL++;

                    FLAGipcResponsePending = false;
                    break;
                }
            }
        }
        break;
    }
    case IPC_REQUEST_ACK:
    {
        if (FLAGipcResponsePending)
        {
            FLAGipcInternalAckPending = false;
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