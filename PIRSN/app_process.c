#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "em_chip.h"
#include "app_log.h"
#include "poll.h"
#include "em_iadc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_burtc.h"
#include "em_acmp.h"
#include "em_lesense.h"
#include "em_pcnt.h"
#include "sl_app_common.h"
#include "app_process.h"
#include "app_framework_common.h"
#include "app_radio.h"
#include "app_init.h"
#include "sl_simple_led_instances.h"

volatile DeviceInfo selfInfo;
volatile bool starttx = false;
volatile bool endtx = false;

EmberEventControl *report_control;
uint16_t sensor_report_period_ms =  (5 * MILLISECOND_TICKS_PER_SECOND);

/*!       IADC_IRQHandler :: ISR
   @brief IADC interrupt handler, called on PRS radio trigger

   @param void

   @return void
*/
void IADC_IRQHandler(void){
  static volatile IADC_Result_t sample;
  sample = IADC_pullSingleFifoResult(IADC0);
  selfInfo.battery_voltage = (sample.data * 1200)/1000;
  IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
}

/*!       GPIO_ODD_IRQHandler :: ISR
   @brief GPIO (ODD) interrupt handler

   @param void

   @return void
*/
void GPIO_ODD_IRQHandler(void)
{
  volatile uint32_t interruptMask = GPIO_IntGet ();
  GPIO_IntClear (interruptMask);
  if ((interruptMask & ((1 << 5) | GPIO_IEN_EM4WUIEN7)) && selfInfo.trigd == 0 &&!firsttrig)
    {
      selfInfo.trigd++;
      selfInfo.state = S_ALERTING;
      sl_led_turn_on (&sl_led_comms);
      sl_led_turn_on (&sl_led_stat);
      starttx = true;

      CMU_ClockSelectSet (cmuClock_EM4GRPACLK, cmuSelect_ULFRCO);
      CMU_ClockEnable (cmuClock_BURTC, true);
      CMU_ClockEnable (cmuClock_BURAM, true);

      BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
      burtcInit.compare0Top = true; // reset counter when counter reaches compare value
      burtcInit.em4comp = false; // BURTC compare interrupt wakes from EM4 (causes reset)
      BURTC_Init (&burtcInit);
      BURTC_CounterReset ();
      BURTC_CompareSet (0, 2600);
      BURTC_IntEnable (BURTC_IEN_COMP);    // compare match
      NVIC_SetPriority (BURTC_IRQn, 4);
      NVIC_EnableIRQ (BURTC_IRQn);
      BURTC_Enable (true);
    }
  else if ((interruptMask & ((1 << 5) | GPIO_IEN_EM4WUIEN7))
      && selfInfo.trigd > 0)
    {
      sl_led_toggle (&sl_led_comms);
      sl_led_toggle (&sl_led_stat);
      selfInfo.trigd++;
      BURTC_CounterReset ();
    }
  if(firsttrig){
      firsttrig = false;
  }

}

/*!       BURTC_IRQHandler :: ISR
   @brief BURTC interrupt handler, used for sensor timeout masking

   @param void

   @return void
*/
void BURTC_IRQHandler(void)
{
  NVIC_ClearPendingIRQ (GPIO_ODD_IRQn);
  NVIC_DisableIRQ (GPIO_ODD_IRQn);
  if (selfInfo.trigd > 0)
    {
      sl_led_toggle (&sl_led_comms);
      sl_led_toggle (&sl_led_stat);
      BURTC_CounterReset ();
      BURTC_CompareSet (0, 1300);
      BURTC_IntEnable (BURTC_IEN_COMP);
      BURTC_IntClear (BURTC_IntGet ());
      NVIC_EnableIRQ (BURTC_IRQn);
      BURTC_Enable (true);
      selfInfo.state = S_ACTIVE;
      endtx = true;
      sl_led_turn_off (&sl_led_comms);
      sl_led_turn_off (&sl_led_stat);
    }
  else
    {
      BURTC_CounterReset ();
      BURTC_CompareSet (0, 2600);
      NVIC_DisableIRQ (BURTC_IRQn);
      BURTC_Enable (false);
      BURTC_IntClear (BURTC_IF_COMP);
      NVIC_ClearPendingIRQ (GPIO_ODD_IRQn);
      NVIC_SetPriority(GPIO_ODD_IRQn, 5);
      NVIC_EnableIRQ (GPIO_ODD_IRQn);

    }
  if(coldstart){
     coldstart = false;

     selfInfo.state = S_INACTIVE;
  }
}

/*!       report_handler :: HANDLER
   @brief tx routine every sensor_report_period_ms

   @param void

   @return void
*/
void report_handler(void)
{
  if (!emberStackIsUp()) {
    emberEventControlSetInactive(*report_control);
  } else {
      sl_led_turn_on (&sl_led_stat);
      applicationSensorTxRoutine();
      emberEventControlSetDelayMS(*report_control, sensor_report_period_ms);
  }
}

bool emberAfCommonOkToEnterLowPowerCallback(bool enter_em2, uint32_t duration_ms)
{
  (void) enter_em2;
  (void) duration_ms;
  return true;
}

void emberAfIncomingMessageCallback (EmberIncomingMessage *message)
{
  sl_led_turn_on (&sl_led_stat);
  applicationSensorRxMsg(&(*message));
}

void emberAfMessageSentCallback(EmberStatus status,
                                EmberOutgoingMessage *message)
{
  (void) message;
  if (status != EMBER_SUCCESS) {
  }
  sl_led_turn_off(&sl_led_comms);
  sl_led_turn_off(&sl_led_stat);
}


void emberAfStackStatusCallback(EmberStatus status)
{
  switch (status)
    {
    case EMBER_NETWORK_UP:
      initSensorInfo (&selfInfo, HW_PIRSN, S_COLDSTART, 0, emberGetNodeType(),
      EMBER_COORDINATOR_ADDRESS,
                      emberGetNodeId(), SENSOR_SINK_ENDPOINT, 0);
      emberEventControlSetDelayMS(*report_control, sensor_report_period_ms);
      startBatteryMonitor ();
      sl_led_turn_on (&sl_led_stat);
      applicationSensorTxInit ();
      sl_led_turn_on (&sl_led_comms);
      break;
    case EMBER_NETWORK_DOWN:
      {
        sl_led_toggle (&sl_led_comms);
        sl_led_toggle (&sl_led_stat);
        status = applicationSensorRadioInit ();
      }
      break;
    case EMBER_JOIN_SCAN_FAILED:
      break;
    case EMBER_JOIN_DENIED:
      break;
    case EMBER_JOIN_TIMEOUT:

      {
        sl_led_toggle (&sl_led_comms);
        sl_led_toggle (&sl_led_stat);
        status = applicationSensorRadioInit ();
      }
      break;
    default:
      {
        sl_led_toggle (&sl_led_comms);
        sl_led_toggle (&sl_led_stat);
        status = applicationSensorRadioInit ();
      }
      break;
    }
}

void emberAfTickCallback(void)
{
  if(starttx){
      applicationSensorTxStartEvent ();
      starttx = false;
  }
  else if(endtx){
      applicationSensorTxEndEvent (selfInfo.trigd);
      selfInfo.trigd = 0;
      endtx = false;
  }
}

/*!       initSensorInfo :: FUNCTION
   @brief setter for DeviceInfo

   @param *info and its input params

   @return void
*/
void initSensorInfo(volatile DeviceInfo *info, device_hw_t hw, sensor_state_t state, uint32_t battery_voltage, EmberNodeType node_type,
                    EmberNodeId central_id, EmberNodeId self_id, uint8_t endpoint, uint8_t trigd) {
  info->hw = hw;
  info->state = state;
  info->battery_voltage = battery_voltage;
  info->node_type = node_type;
  info->central_id = central_id;
  info->self_id = self_id;
  info->endpoint = endpoint;
  info->trigd = trigd;
}

