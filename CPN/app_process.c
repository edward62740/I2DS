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

void emberAfIncomingMessageCallback (EmberIncomingMessage *message)
{
  sl_led_turn_on(&sl_led_stat);
  applicationCoordinatorRxMsg(&(*message));
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
  app_log_info("sent ... \n" );
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
