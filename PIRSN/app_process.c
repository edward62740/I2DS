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
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "em_chip.h"
#include "app_log.h"
#include "sl_si70xx.h"
#include "sl_i2cspm_instances.h"
#include "poll.h"
#include "em_iadc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "sl_app_common.h"
#include "app_process.h"
#include "app_framework_common.h"
#if defined(SL_CATALOG_LED0_PRESENT)
#include "sl_simple_led_instances.h"
#endif
#include "sl_simple_button_instances.h"
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_component_catalog.h"
#include "sl_power_manager.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define MAX_TX_FAILURES     (10u)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Global flag set by a button push to allow or disallow entering to sleep
bool enable_sleep = false;
/// report timing event control
EmberEventControl *report_control;
/// report timing period
uint16_t sensor_report_period_ms =  (5 * MILLISECOND_TICKS_PER_SECOND);
/// TX options set up for the network
EmberMessageOptions tx_options = EMBER_OPTIONS_ACK_REQUESTED | EMBER_OPTIONS_SECURITY_ENABLED;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Destination of the currently processed sink node
static EmberNodeId sink_node_id = EMBER_COORDINATOR_ADDRESS;
static volatile IADC_Result_t sample;
uint8_t buffer[4];

void IADC_IRQHandler(void)
{
  // Read a result from the FIFO
  sample = IADC_pullSingleFifoResult(IADC0);

  /*
   * Calculate the voltage converted as follows:
   *
   * For single-ended conversions, the result can range from 0 to
   * +Vref, i.e., for Vref = AVDD = 3.30 V, 0xFFF represents the
   * full scale value of 3.30 V.
   */
  uint32_t data = (sample.data * 1200)/1000;
  app_log_info(" %d", data);
  buffer[0] = 0xFF & (uint8_t)(data >> 24);
  buffer[1] = 0xFF & (uint8_t)(data >> 16);
  buffer[2] = 0xFF & (uint8_t)(data >> 8);
  buffer[3] = 0xFF & (uint8_t)(data);

  /*
   * Clear the single conversion complete interrupt.  Reading FIFO
   * results does not do this automatically.
   */
  IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
      uint8_t buf[1];
          buf[0] = 0x01;
          emberMessageSend(sink_node_id,
                                          SENSOR_SINK_ENDPOINT, // endpoint
                                          0, // messageTag
                                          sizeof(buf),
                                          buf,
                                          tx_options);

  }
}

/**************************************************************************//**
 * Here we print out the first two bytes reported by the sinks as a little
 * endian 16-bits decimal.
 *****************************************************************************/
void report_handler(void)
{
  if (!emberStackIsUp()) {
    emberEventControlSetInactive(*report_control);
  } else {
      emberEventControlSetDelayMS(*report_control, sensor_report_period_ms);

      app_log_info("poll");
      NVIC_DisableIRQ(IADC_IRQn);
      emberPollForData();
      NVIC_ClearPendingIRQ(IADC_IRQn);
            NVIC_EnableIRQ(IADC_IRQn);
               emberMessageSend(sink_node_id,
                                                        SENSOR_SINK_ENDPOINT, // endpoint
                                                        0, // messageTag
                                                        sizeof(buffer),
                                                        buffer,
                                                        tx_options);
     // IADC_command(IADC0, iadcCmdStartSingle);
  }
}

/**************************************************************************//**
 * Entering sleep is approved or denied in this callback, depending on user
 * demand.
 *****************************************************************************/
bool emberAfCommonOkToEnterLowPowerCallback(bool enter_em2, uint32_t duration_ms)
{
  (void) enter_em2;
  (void) duration_ms;
  return true;
}

/**************************************************************************//**
 * This function is called when a message is received.
 *****************************************************************************/
void emberAfIncomingMessageCallback(EmberIncomingMessage *message)
{
  uint8_t i;
  if (message->endpoint == SENSOR_SINK_ENDPOINT) {
    app_log_info("RX: Data from 0x%04X:", message->source);
    for (i = SENSOR_SINK_DATA_OFFSET; i < message->length; i++) {
      app_log_info(" %x", message->payload[i]);
    }
    app_log_info("\n");
  }
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
 * This function is called when the stack status changes.
 *****************************************************************************/
void emberAfStackStatusCallback(EmberStatus status)
{
  switch (status) {
    case EMBER_NETWORK_UP:
      app_log_info("Network up\n");
      app_log_info("Joined to Sink with node ID: 0x%04X\n", emberGetNodeId());
      // Schedule start of periodic sensor reporting to the Sink
      emberEventControlSetDelayMS(*report_control, sensor_report_period_ms);
      break;
    case EMBER_NETWORK_DOWN:
      app_log_info("Network down\n");
      break;
    case EMBER_JOIN_SCAN_FAILED:
      app_log_error("Scanning during join failed\n");
      break;
    case EMBER_JOIN_DENIED:
      app_log_error("Joining to the network rejected!\n");
      break;
    case EMBER_JOIN_TIMEOUT:
      app_log_info("Join process timed out!\n");
      break;
    default:
      app_log_info("Stack status: 0x%02X\n", status);
          EmberNetworkParameters parameters;
        MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
        parameters.radioTxPower = 0;
        parameters.radioChannel = 11;
        parameters.panId = 0x01FF;
        status = emberJoinNetwork(EMBER_STAR_SLEEPY_END_DEVICE, &parameters);
        app_log_info("Network status 0x%02X\n", status);
      break;
  }
}

/**************************************************************************//**
 * This callback is called in each iteration of the main application loop and
 * can be used to perform periodic functions.
 *****************************************************************************/
void emberAfTickCallback(void)
{
#if defined(SL_CATALOG_LED0_PRESENT)
  if (emberStackIsUp()) {
   // sl_led_turn_on(&sl_led_led0);
  } else {
    sl_led_turn_off(&sl_led_led0);
  }
#endif
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