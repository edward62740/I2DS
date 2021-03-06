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
#include "app_radio.h"

DeviceInfo selfInfo, sensorInfo[MAX_CONNECTED_DEVICES];
uint8_t sensorIndex = 0;


void emberAfIncomingMessageCallback (EmberIncomingMessage *message)
{
  sl_led_turn_on(&sl_led_stat);
  applicationCoordinatorRxMsg(&(*message));
  sl_led_turn_off(&sl_led_stat);
}

void emberAfMessageSentCallback(EmberStatus status,
                                EmberOutgoingMessage *message)
{
  (void) message;
  app_log_info("sent ... \n" );
  if (status != EMBER_SUCCESS) {
    app_log_info("TX: 0x%02X\n", status);
  }
}

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
              parameters.radioTxPower = 200;
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
        parameters.radioTxPower = 200;
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

void emberAfChildJoinCallback(EmberNodeType nodeType,
                              EmberNodeId nodeId)
{
  app_log_info("Sensor joined with node ID 0x%04X, node type: 0x%02X\n", nodeId, nodeType);
}

void emberAfTickCallback(void)
{

}

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
