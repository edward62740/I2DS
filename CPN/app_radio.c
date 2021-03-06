#include PLATFORM_HEADER
#include "app_process.h"
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "em_chip.h"
#include "app_log.h"
#include "sl_app_common.h"
#include "app_framework_common.h"
#include "sl_simple_led_instances.h"
#include "app_process.h"
#include "app_ipc.h"

EmberMessageOptions tx_options = EMBER_OPTIONS_ACK_REQUESTED | EMBER_OPTIONS_SECURITY_ENABLED;


EmberStatus applicationCoordinatorTxSentinel(EmberNodeId source, EmberNodeId dest)
{
  uint8_t buffer[4];
  buffer[0] = 0xFF & (uint8_t) MSG_WARN;
  buffer[1] = 0xFF & (uint8_t) 1;
  buffer[2] = 0xFF & (uint8_t) source << 8;
  buffer[3] = 0xFF & (uint8_t) source;

  return emberMessageSend (dest,
  SENSOR_SINK_ENDPOINT, // endpoint
                    0, // messageTag
                    sizeof(buffer), buffer, tx_options);
}


bool applicationCoordinatorTxRequest(EmberNodeId id, message_request_t type, uint8_t val)
{
  EmberStatus status;
  uint8_t buffer[3];
  buffer[0] = 0xFF & (uint8_t) MSG_REQUEST;
  buffer[1] = 0xFF & (uint8_t) type;
  buffer[2] = 0xFF & (uint8_t) val;
  return (emberMessageSend (id,
   SENSOR_SINK_ENDPOINT, // endpoint
                    0, // messageTag
                    sizeof(buffer), buffer, tx_options) == 0x00) ? true : false;
}

bool applicationCoordinatorTxIdentify(EmberNodeId id)
{
  app_log_info("request init device %d", id);
  EmberStatus status;
  uint8_t buffer[1];
  buffer[0] = 0xFF & (uint8_t) MSG_SYNC;
  return (emberMessageSend (id,
   SENSOR_SINK_ENDPOINT, // endpoint
                    0, // messageTag
                    sizeof(buffer), buffer, tx_options) == 0x00) ? true : false;
}



void applicationCoordinatorRxMsg(EmberIncomingMessage *message)
{
  if ((message->endpoint != SENSOR_SINK_ENDPOINT)
        || ((tx_options & EMBER_OPTIONS_SECURITY_ENABLED)
            && !(message->options & EMBER_OPTIONS_SECURITY_ENABLED))) {
      return;
    }
    switch (message->payload[0])
      {
      case MSG_REPORT:
        {
        uint8_t iter = 0;
        for (uint8_t i = 0; i < (uint8_t) MAX_CONNECTED_DEVICES; i++)
          {
            if (sensorInfo[i].self_id == message->source)
              {
                sensorInfo[i].battery_voltage = message->payload[1] << 24
                    | message->payload[2] << 16 | message->payload[3] << 8
                    | message->payload[4];
                sensorInfo[i].state = message->payload[5];
                sensorInfo[i].rssi = message->rssi;
                sensorInfo[i].lqi = message->lqi;
                ipcReport(message->source, sensorInfo[i].battery_voltage, sensorInfo[i].state, sensorInfo[i].rssi, sensorInfo[i].lqi);
                break;
              }
            else
              {
                iter++;
              }
          }
        if (iter == (uint8_t) MAX_CONNECTED_DEVICES)
          {
            applicationCoordinatorTxIdentify(message->source);
          }
        break;
        }
      case MSG_REPLY:
      {
        for (uint8_t i = 0; i < (uint8_t) MAX_CONNECTED_DEVICES; i++)
          {
            if (sensorInfo[i].self_id == message->source)
              {
                sensorInfo[i].state = message->payload[2];
                break;
              }
          }
          ipcRequestDone(message->payload[1],message->source,message->payload[2]);
        app_log_info(" Ack \n");
        break;
        }
      case MSG_WARN:
        {

          if(message->payload[1] == 0){
              app_log_info(" Warning started \n");
              ipcNotify(message->source, message->payload[3], 0x00, 0x00);
          }
          else {
              app_log_info(" Warning ended \n");
              ipcNotify(message->source, message->payload[3], 0x01, message->payload[2]);
          }
        for (uint8_t i = 0; i < (uint8_t) MAX_CONNECTED_DEVICES; i++)
          {
            if (sensorInfo[i].self_id == message->source)
              {
                sensorInfo[i].state = message->payload[3];
                sensorInfo[i].rssi = message->rssi;
                sensorInfo[i].lqi = message->lqi;
                break;
              }
          }
        for (uint8_t i = 0; i < (uint8_t) MAX_CONNECTED_DEVICES; i++)
          {
            if (sensorInfo[i].hw == (uint8_t)HW_SENTINEL)
              {
                applicationCoordinatorTxSentinel(message->source, sensorInfo[i].self_id);
                break;
              }
          }
          break;
        }
      case MSG_INIT:
        {
          uint8_t hw = message->payload[1];
          uint8_t state = message->payload[2];
          uint32_t battery = message->payload[3] << 24;
          battery |= message->payload[4] << 16;
          battery |= message->payload[5] << 8;
          battery |= message->payload[6];
          uint8_t nt = message->payload[7];
          uint16_t cid = message->payload[8] << 8;
          cid |= message->payload[9];
          uint16_t sid = message->payload[10] << 8;
          sid |= message->payload[11];
          uint8_t ep = message->payload[12];
          uint8_t iter = 0;
          for (uint8_t i = 0; i < (uint8_t)MAX_CONNECTED_DEVICES; i++)
            {
              if (sensorInfo[i].self_id == message->source)
                {
                  sensorInfo[i].hw = hw;
                  sensorInfo[i].state = state;
                  sensorInfo[i].battery_voltage = battery;
                  sensorInfo[i].node_type = nt;
                  sensorInfo[i].central_id = cid;
                  sensorInfo[i].self_id = sid;
                  sensorInfo[i].endpoint = ep;
                  sensorInfo[i].rssi = message->rssi;
                  sensorInfo[i].lqi = message->lqi;
                  app_log_info("Updated device %d ", sid);
                  break;
                }
              else
                {
                  iter++;
                }
            }
          if(iter == (uint8_t)MAX_CONNECTED_DEVICES){
              sensorInfo[sensorIndex].hw = hw;
              sensorInfo[sensorIndex].state = state;
              sensorInfo[sensorIndex].battery_voltage = battery;
              sensorInfo[sensorIndex].node_type = nt;
              sensorInfo[sensorIndex].central_id = cid;
              sensorInfo[sensorIndex].self_id = sid;
              sensorInfo[sensorIndex].endpoint = ep;
              sensorInfo[sensorIndex].rssi = message->rssi;
              sensorInfo[sensorIndex].lqi = message->lqi;
              sensorIndex++;
              app_log_info("Added new device %d ", sid);
              ipcNotify(message->source, message->payload[2], 0x02, 0xFF);
          }
          app_log_info(
              " INIT -> State: %d, HW: %d, Voltage: %d, Nodetype: %d, CID: %d, SID: %d, ENDP: %d \n",
              state, hw, battery, nt, cid, sid, ep);
          break;
        }
      }
}
