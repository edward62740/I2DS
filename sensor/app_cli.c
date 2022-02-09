/***************************************************************************//**
 * @file
 * @brief app_cli.c
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
#include <string.h>
#include PLATFORM_HEADER
#include "em_chip.h"
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "sl_cli.h"
#include "app_log.h"
#include "sl_app_common.h"
#include "stack-info.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define ENABLED  "enabled"
#define DISABLED "disabled"

// The destination endpoint of the outgoing message
#define DATA_ENDPOINT           1
#define TX_TEST_ENDPOINT        2

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static bool check_channel(uint16_t channel);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
/// Connect security key set up by CLI (or the default key)
EmberKeyData security_key = { .contents = SENSOR_SINK_SECURITY_KEY };

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Sensor TX power set by CLI command
static int16_t tx_power = SENSOR_SINK_TX_POWER;

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
 * Checks if the current channel is valid for the selected PHY.
 * If the channel is invalid, it recommends the first available channel.
 * Returns false if the given channel is below the first allowed channel
 * or true otherwise.
 *****************************************************************************/
static bool check_channel(uint16_t channel)
{
  bool channel_ok = true;
  uint16_t default_channel = emberGetDefaultChannel();

  if (channel < default_channel) {
    app_log_info("Channel %d is invalid, the first valid channel is %d!\n", channel, default_channel);
    channel_ok = false;
  }
  return (channel_ok);
}

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

/******************************************************************************
 * CLI - Join as End Device
 * Joins the network on the specified channel.
 *****************************************************************************/
void cli_join(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  EmberNetworkParameters parameters;
  uint16_t channel = sl_cli_get_argument_uint8(arguments, 0);
  // Abort if the channel is invalid for this PHY
  if (check_channel(channel) == false) {
    return;
  }

  size_t length = 0;
  uint8_t *contents = NULL;

  // Initialize the security key to the default key prior to joining the
  // network.
  emberSetSecurityKey(&security_key);

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = tx_power;
  parameters.radioChannel = channel;

  // set default PAN ID or the one passed as parameter
  if (sl_cli_get_argument_count(arguments) > 1) {
    parameters.panId = sl_cli_get_argument_uint16(arguments, 1);
  } else {
    parameters.panId = SENSOR_SINK_PAN_ID;
  }

  emberClearSelectiveJoinPayload();

  status = emberJoinNetwork(EMBER_STAR_END_DEVICE, &parameters);
  app_log_info("join end device: status 0x%02X\n", status);
}

/******************************************************************************
 * CLI - Join as Sleepy End Device
 * Joins the network on the specified channel.
 *****************************************************************************/
void cli_join_sleepy(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  EmberNetworkParameters parameters;
  uint16_t channel = sl_cli_get_argument_uint8(arguments, 0);
  // Abort if the channel is invalid for this PHY
  if (check_channel(channel) == false) {
    return;
  }
  // Initialize the security key to the default key prior to joining the
  // network.
  emberSetSecurityKey(&security_key);

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = tx_power;
  parameters.radioChannel = channel;

  // set default PAN ID or the one passed as parameter
  if (sl_cli_get_argument_count(arguments) > 1) {
    parameters.panId = sl_cli_get_argument_uint16(arguments, 1);
  } else {
    parameters.panId = SENSOR_SINK_PAN_ID;
  }

  status = emberJoinNetwork(EMBER_STAR_SLEEPY_END_DEVICE, &parameters);
  app_log_info("join sleepy 0x%02X\n", status);
}

/******************************************************************************
 * CLI - Join as Range Extender
 * Joins the network on the specified channel.
 *****************************************************************************/
void cli_join_extender(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  EmberNetworkParameters parameters;
  uint16_t channel = sl_cli_get_argument_uint8(arguments, 0);
  // Abort if the channel is invalid for this PHY
  if (check_channel(channel) == false) {
    return;
  }
  // Initialize the security key to the default key prior to joining the
  // network.
  emberSetSecurityKey(&security_key);

  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = tx_power;
  parameters.radioChannel = channel;

  // set default PAN ID or the one passed as parameter
  if (sl_cli_get_argument_count(arguments) > 1) {
    parameters.panId = sl_cli_get_argument_uint16(arguments, 1);
  } else {
    parameters.panId = SENSOR_SINK_PAN_ID;
  }

  status = emberJoinNetwork(EMBER_STAR_RANGE_EXTENDER, &parameters);
  app_log_info("join range extender 0x%02X\n", status);
}

/******************************************************************************
 * CLI - permit join
 * Allows sensors to join a range extender for a given seconds (or unlimited = 0xff)
 *****************************************************************************/
void cli_pjoin(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  uint8_t duration = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
  size_t length = 0;
  uint8_t *contents = NULL;

  if (sl_cli_get_argument_count(arguments) > 1) {
    contents = sl_cli_get_argument_hex(arguments, 1, &length);
    status = emberSetSelectiveJoinPayload(length, contents);
    if (status != EMBER_SUCCESS) {
      app_log_warning("Join Payload status:%d ", status);
    }
  } else {
    emberClearSelectiveJoinPayload();
  }

  emberPermitJoining(duration);
}

/******************************************************************************
 * CLI - set TX options
 * Set the option bits for ACKs, security and priority via the binary value given.
 *****************************************************************************/
void cli_set_tx_options(sl_cli_command_arg_t *arguments)
{
  tx_options = sl_cli_get_argument_uint8(arguments, 0);
  app_log_info("TX options set: MAC acks %s, security %s, priority %s\n",
               ((tx_options & EMBER_OPTIONS_ACK_REQUESTED) ? "enabled" : "disabled"),
               ((tx_options & EMBER_OPTIONS_SECURITY_ENABLED) ? "enabled" : "disabled"),
               ((tx_options & EMBER_OPTIONS_HIGH_PRIORITY) ? "enabled" : "disabled"));;
}

/******************************************************************************
 * CLI - set TX power
 * Set antenna power in 0.1 dBm resolution
 *****************************************************************************/
void cli_set_tx_power(sl_cli_command_arg_t *arguments)
{
  bool save_power = false;
  tx_power = sl_cli_get_argument_int16(arguments, 0);

  // check optional parameter for storing the value we're about to set
  if (sl_cli_get_argument_count(arguments) > 1) {
    save_power =  sl_cli_get_argument_int8(arguments, 1);
  }

  if (emberSetRadioPower(tx_power, save_power) == EMBER_SUCCESS) {
    app_log_info("TX power set: %d\n", (int16_t)emberGetRadioPower());
  } else {
    app_log_error("TX power set failed\n");
  }
}

/******************************************************************************
 * CLI - set_security_key command
 * The command sets the security key
 *****************************************************************************/
void cli_set_security_key(sl_cli_command_arg_t *arguments)
{
  size_t key_len = 0;
  const char* const key_input_string = sl_cli_get_argument_hex(arguments, 0, &key_len);
  if (key_len == EMBER_ENCRYPTION_KEY_SIZE) {
    EmberKeyData key;
    memcpy(key.contents, key_input_string, key_len);
    if (emberSetSecurityKey(&key) == EMBER_SUCCESS) {
      app_log_info("Security key set: {");
      for (uint8_t i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
        if (i == (EMBER_ENCRYPTION_KEY_SIZE - 1)) {
          app_log_info("%02X", key.contents[i]);
        } else {
          app_log_info("%02X ", key.contents[i]);
        }
      }
      app_log_info("}\n");
    } else {
      app_log_error("Security key set failed\n");
    }
  } else {
    app_log_info("Security key length must be: %d bytes\n", EMBER_ENCRYPTION_KEY_SIZE);
  }
}

/******************************************************************************
 * CLI - Set Report Period
 *
 *****************************************************************************/
void cli_set_report_period(sl_cli_command_arg_t *arguments)
{
  sensor_report_period_ms = sl_cli_get_argument_uint16(arguments, 0);

  app_log_info("Report period set to %d ms\n", sensor_report_period_ms);
}

/******************************************************************************
 * CLI - advertise
 * Force sensor to send an advertise request to the sink.
 *****************************************************************************/
void cli_advertise(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  request_advertise();
}

/******************************************************************************
 * CLI - data
 * Force sensor to send a report to the sink.
 *****************************************************************************/
void cli_data(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  emberEventControlSetActive(*report_control);
}

/******************************************************************************
 * CLI - info command
 * It lists the main attributes of the current state of the node
 *****************************************************************************/
void cli_info(sl_cli_command_arg_t *arguments)
{
  (void) arguments;

  uint8_t* eui64 = emberGetEui64();

  char* is_ack = ((tx_options & EMBER_OPTIONS_ACK_REQUESTED) ? ENABLED : DISABLED);
  char* is_security = ((tx_options & EMBER_OPTIONS_SECURITY_ENABLED) ? ENABLED : DISABLED);
  char* is_high_prio = ((tx_options & EMBER_OPTIONS_HIGH_PRIORITY) ? ENABLED : DISABLED);

  app_log_info("Info:\n");
  app_log_info("         MCU Id: 0x%llX\n", SYSTEM_GetUnique());
  app_log_info("  Network state: 0x%02X\n", emberNetworkState());
  app_log_info("      Node type: 0x%02X\n", emberGetNodeType());
  app_log_info("          eui64: >%x%x%x%x%x%x%x%x\n",
               eui64[7],
               eui64[6],
               eui64[5],
               eui64[4],
               eui64[3],
               eui64[2],
               eui64[1],
               eui64[0]);
  app_log_info("        Node id: 0x%04X\n", emberGetNodeId());
  app_log_info("         Pan id: 0x%04X\n", emberGetPanId());
  app_log_info("        Channel: %d\n", (uint16_t)emberGetRadioChannel());
  app_log_info("          Power: %d\n", (int16_t)emberGetRadioPower());
  app_log_info("     TX options: MAC acks %s, security %s, priority %s\n", is_ack, is_security, is_high_prio);
}

/******************************************************************************
 * CLI - counter command
 * The command prints out the passed stack counter
 *****************************************************************************/
void cli_counter(sl_cli_command_arg_t *arguments)
{
  uint8_t counter_type = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t counter;
  EmberStatus status = emberGetCounter(counter_type, &counter);

  if (status == EMBER_SUCCESS) {
    app_log_info("Counter type=0x%02X: %lu\n", counter_type, counter);
  } else {
    app_log_error("Get counter failed, status=0x%02X\n", status);
  }
}

/******************************************************************************
 * CLI - start_energy_scan command
 * The command scans the energy level on the given channel, e.g.
 * "start_energy_scan 0 10" results in 10 RSSI samples collected on channel 0.
 *****************************************************************************/
void cli_start_energy_scan(sl_cli_command_arg_t *arguments)
{
  EmberStatus status;
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t sample_num = sl_cli_get_argument_uint8(arguments, 1);
  status = emberStartEnergyScan(channel, sample_num);

  if (status == EMBER_SUCCESS) {
    app_log_info("Start energy scanning: channel %d, samples %d\n", channel, sample_num);
  } else {
    app_log_error("Start energy scanning failed, status=0x%02X\n", status);
  }
}

/******************************************************************************
 * CLI - leave command
 * By this API call the node forgets the current network and reverts to
 * a network status of EMBER_NO_NETWORK
 *****************************************************************************/
void cli_leave(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  emberResetNetworkState();
}

/******************************************************************************
 * CLI - set_tx_options command
 * Sets the tx options
 *****************************************************************************/
void cli_set_tx_option(sl_cli_command_arg_t *arguments)
{
  tx_options = sl_cli_get_argument_uint8(arguments, 0);
  char* is_ack = ((tx_options & EMBER_OPTIONS_ACK_REQUESTED) ? ENABLED : DISABLED);
  char* is_security = ((tx_options & EMBER_OPTIONS_SECURITY_ENABLED) ? ENABLED : DISABLED);
  char* is_high_prio = ((tx_options & EMBER_OPTIONS_HIGH_PRIORITY) ? ENABLED : DISABLED);

  app_log_info("TX options set: MAC acks %s, security %s, priority %s", is_ack, is_security, is_high_prio);
}

/******************************************************************************
 * CLI - reset command
 * Resets the hardware
 *****************************************************************************/
void cli_reset(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  halReboot();
}
