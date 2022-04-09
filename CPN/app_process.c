/***************************************************************************//**
 * @file
 * @brief app_process.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
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
#include "em_cmu.h"
#include "em_emu.h"
#include "em_eusart.h"
#include "app_ipc.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

DeviceInfo selfInfo, sensorInfo[MAX_CONNECTED_DEVICES];
uint8_t sensorIndex = 0;

void initSensorInfo(DeviceInfo *info, device_hw_t hw, sensor_state_t state, uint32_t battery_voltage, EmberNodeType node_type,
                    EmberNodeId central_id, EmberNodeId self_id, uint8_t endpoint) {
  info->hw = hw;
  info->state = state;
  info->battery_voltage = battery_voltage;
  info->node_type = node_type;
  info->central_id = central_id;
  info->self_id = self_id;
  info->endpoint = endpoint;
}
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// TX options set up for the network
EmberMessageOptions tx_options = EMBER_OPTIONS_ACK_REQUESTED | EMBER_OPTIONS_SECURITY_ENABLED;


// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * This function is called when a message is received.
 *****************************************************************************/

uint8_t remote_state = S_INACTIVE;






void emberAfIncomingMessageCallback (EmberIncomingMessage *message)
{
  sl_led_turn_on(&sl_led_stat);
  if ((message->endpoint != SENSOR_SINK_ENDPOINT)
      || ((tx_options & EMBER_OPTIONS_SECURITY_ENABLED)
          && !(message->options & EMBER_OPTIONS_SECURITY_ENABLED))) {
    // drop the message if it's not coming from a sensor
    // or if security is required but the message is non-encrypted
    return;
  }
  uint8_t buffer[message->length];
  //app_log_info("RX: Data from 0x%04X:", message->source);
  //app_log_info("rssi: %d, lqi: %d", message->rssi, message->lqi);
  for (int j = 0; j < message->length; j++) {
    //app_log_info(" %02X", message->payload[j]);
    buffer[j] = message->payload[j];
  }
  switch (buffer[0])
    {
    case MSG_REPORT:
      {
        uint32_t data = buffer[1] << 24;
        data |= buffer[2] << 16;
        data |= buffer[3] << 8;
        data |= buffer[4];
        uint8_t state = buffer[5];
        remote_state = state;
        uint8_t iter = 0;
        for (uint8_t i = 0; i < (uint8_t) MAX_CONNECTED_DEVICES; i++)
          {
            if (sensorInfo[i].self_id == message->source)
              {
                sensorInfo[i].battery_voltage = data;
                break;
              }
            else
              {
                iter++;
              }
          }
        app_log_info(" Voltage: %d, State: %d \n", data, state);
      break;
      }
    case MSG_REPLY:
      {
        uint8_t info = buffer[1];
      app_log_info(" Ack state switch, State: %d \n", info);
      break;
      }
    case MSG_WARN:
      {
        uint8_t info = buffer[1];
        if(info == 0){
            app_log_info(" Warning started %d \n", info);
        }
        else {
            app_log_info(" Warning ended with %d trigs \n", buffer[2]);
        }

        uint8_t tx_buffer[3];
        tx_buffer[0] = (uint8_t) MSG_REQUEST;
        tx_buffer[1] = (uint8_t) REQ_STATE;
        if (remote_state == S_INACTIVE)
          {
            tx_buffer[2] = (uint8_t) S_ACTIVE;
          }
        else if (remote_state == S_ACTIVE)
          {
            tx_buffer[2] = (uint8_t) S_INACTIVE;
          }
/*
        emberMessageSend (0x0001,
        SENSOR_SINK_ENDPOINT, // endpoint
                          0, // messageTag
                          sizeof(tx_buffer), tx_buffer, tx_options);
        app_log_info(" Request state switch to %d \n", tx_buffer[2]);
        */
        break;
      }
    case MSG_INIT:
      {
        uint8_t hw = buffer[1];
        uint8_t state = buffer[2];
        uint32_t battery = buffer[3] << 24;
        battery |= buffer[4] << 16;
        battery |= buffer[5] << 8;
        battery |= buffer[6];
        uint8_t nt = buffer[7];
        uint16_t cid = buffer[8] << 8;
        cid |= buffer[9];
        uint16_t sid = buffer[10] << 8;
        sid |= buffer[11];
        uint8_t ep = buffer[12];
        uint8_t iter = 0;
        for (uint8_t i = 0; i < (uint8_t)MAX_CONNECTED_DEVICES; i++)
          {
            if (sensorInfo[i].self_id == message->source)
              {
                sensorInfo[i].hw = buffer[1];
                sensorInfo[i].state = buffer[2];
                sensorInfo[i].battery_voltage = battery;
                sensorInfo[i].node_type = buffer[7];
                sensorInfo[i].central_id = cid;
                sensorInfo[i].self_id = sid;
                sensorInfo[i].endpoint = buffer[12];
                app_log_info("Updated device %d ", sid);
                break;
              }
            else
              {
                iter++;
              }
          }
        app_log_info("icnt %d ", iter);
        if(iter == (uint8_t)MAX_CONNECTED_DEVICES){
            sensorInfo[sensorIndex].hw = buffer[1];
            sensorInfo[sensorIndex].state = buffer[2];
            sensorInfo[sensorIndex].battery_voltage = battery;
            sensorInfo[sensorIndex].node_type = buffer[7];
            sensorInfo[sensorIndex].central_id = cid;
            sensorInfo[sensorIndex].self_id = sid;
            sensorInfo[sensorIndex].endpoint = buffer[12];
            sensorIndex++;
            app_log_info("Added new device %d ", sid);
        }
        remote_state = state;
        app_log_info(
            " INIT -> State: %d, HW: %d, Voltage: %d, Nodetype: %d, CID: %d, SID: %d, ENDP: %d \n",
            state, hw, battery, nt, cid, sid, ep);
        break;
      }

    default:
      break;
    }


  sl_led_turn_off(&sl_led_stat);
}

/**************************************************************************//**
 * This function is called to indicate whether an outgoing message was
 * successfully transmitted or to indicate the reason of failure.
 *****************************************************************************/
void emberAfMessageSentCallback(EmberStatus status,
                                EmberOutgoingMessage *message)
{
  (void) message;
  if (status != EMBER_SUCCESS) {
    app_log_info("TX: 0x%02X\n", status);
  }
}

/**************************************************************************//**
 * This function is called by the application framework from the stack status
 * handler.
 *****************************************************************************/
void emberAfStackStatusCallback(EmberStatus status)
{
  switch (status) {
    case EMBER_NETWORK_UP:
      app_log_info("Network up\n");
      sl_led_turn_on(&sl_led_comms);

      break;
    case EMBER_NETWORK_DOWN:
      {
        sl_led_toggle(&sl_led_comms);
              EmberNetworkParameters parameters;
              MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
              parameters.radioTxPower = SENSOR_SINK_TX_POWER;
              parameters.radioChannel = 11;
              parameters.panId = SENSOR_SINK_PAN_ID;
              emberSetSecurityKey (&security_key);
              while (status != EMBER_SUCCESS)
                {
                  status = emberFormNetwork (&parameters);
                  sl_sleeptimer_delay_millisecond (500);
                  app_log_info("form Network status 0x%02X\n", status);
                }
              status = 0x01;
              sl_led_toggle(&sl_led_comms);
              emberClearSelectiveJoinPayload ();
              while (status != EMBER_SUCCESS)
                {
                  status = emberPermitJoining (255);
                  sl_sleeptimer_delay_millisecond (500);
                  app_log_info("pj Network status 0x%02X\n", status);
                }
              app_log_info("Stack status: 0x%02X\n", status);
              sl_led_toggle(&sl_led_comms);
              break;
            }
      break;
    default:
      {
        sl_led_toggle(&sl_led_comms);
        EmberNetworkParameters parameters;
        MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
        parameters.radioTxPower = SENSOR_SINK_TX_POWER;
        parameters.radioChannel = 11;
        parameters.panId = SENSOR_SINK_PAN_ID;
        emberSetSecurityKey (&security_key);
        while (status != EMBER_SUCCESS)
          {
            status = emberFormNetwork (&parameters);
            sl_sleeptimer_delay_millisecond (500);
            app_log_info("form Network status 0x%02X\n", status);
          }
        status = 0x01;
        sl_led_toggle(&sl_led_comms);
        emberClearSelectiveJoinPayload ();
        while (status != EMBER_SUCCESS)
          {
            status = emberPermitJoining (255);
            sl_sleeptimer_delay_millisecond (500);
            app_log_info("pj Network status 0x%02X\n", status);
          }
        app_log_info("Stack status: 0x%02X\n", status);
        sl_led_toggle(&sl_led_comms);
        break;
      }
    }
}

/**************************************************************************//**
 * This handler is invoked when a new child has joined the device.
 *****************************************************************************/
void emberAfChildJoinCallback(EmberNodeType nodeType,
                              EmberNodeId nodeId)
{
  app_log_info("Sensor joined with node ID 0x%04X, node type: 0x%02X\n", nodeId, nodeType);
}

/**************************************************************************//**
 * This function is called in each iteration of the main application loop and
 * can be used to perform periodic functions.
 *****************************************************************************/
void emberAfTickCallback(void)
{

  if (emberStackIsUp()) {
    //sl_led_turn_off(&sl_led_led0);
  } else {
   // sl_led_turn_on(&sl_led_led0);
  }
}

/**************************************************************************//**
 * This function is called when a frequency hopping client completed the start
 * procedure.
 *****************************************************************************/
void emberAfFrequencyHoppingStartClientCompleteCallback(EmberStatus status)
{
  if (status != EMBER_SUCCESS) {
    app_log_error("FH Client sync failed, status=0x%02X\n", status);
  } else {
    app_log_info("FH Client Sync Success\n");
  }
}

/**************************************************************************//**
 * This function is called when a requested energy scan is complete.
 *****************************************************************************/
void emberAfEnergyScanCompleteCallback(int8_t mean,
                                       int8_t min,
                                       int8_t max,
                                       uint16_t variance)
{
  app_log_info("Energy scan complete, mean=%d min=%d max=%d var=%d\n",
               mean, min, max, variance);
}

#if defined(EMBER_AF_PLUGIN_MICRIUM_RTOS) && defined(EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1)
/**************************************************************************//**
 * This function is called from the Micrium RTOS plugin before the
 * Application (1) task is created.
 *****************************************************************************/
void emberAfPluginMicriumRtosAppTask1InitCallback(void)
{
  app_log_info("app task init\n");
}

#include <kernel/include/os.h>
#define TICK_INTERVAL_MS 1000

/**************************************************************************//**
 * This function implements the Application (1) task main loop.
 *****************************************************************************/
void emberAfPluginMicriumRtosAppTask1MainLoopCallback(void *p_arg)
{
  RTOS_ERR err;
  OS_TICK yield_time_ticks = (OSCfg_TickRate_Hz * TICK_INTERVAL_MS) / 1000;

  while (true) {
    app_log_info("app task tick\n");

    OSTimeDly(yield_time_ticks, OS_OPT_TIME_DLY, &err);
  }
}

#endif // EMBER_AF_PLUGIN_MICRIUM_RTOS && EMBER_AF_PLUGIN_MICRIUM_RTOS_APP_TASK1

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------s
