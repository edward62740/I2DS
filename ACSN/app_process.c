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

  if ((interruptMask & ((1 << 5) | GPIO_IEN_EM4WUIEN7)) && GPIO_PinInGet(gpioPortC, 5))
    {
      sl_led_turn_on (&sl_led_comms);
      sl_led_turn_on (&sl_led_stat);
      selfInfo.trigd++;
      selfInfo.state = S_ALERTING;
      starttx = true;
      GPIO_EM4EnablePinWakeup(GPIO_IEN_EM4WUIEN7, 0 << _GPIO_IEN_EM4WUIEN7_SHIFT);
    }
  else if ((interruptMask & ((1 << 5) | GPIO_IEN_EM4WUIEN7)) && !GPIO_PinInGet(gpioPortC, 5))
    {
      GPIO_EM4DisablePinWakeup (GPIO_IEN_EM4WUIEN7);
      CMU_ClockSelectSet (cmuClock_EM4GRPACLK, cmuSelect_ULFRCO);
      CMU_ClockEnable (cmuClock_BURTC, true);
      CMU_ClockEnable (cmuClock_BURAM, true);

      BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
      burtcInit.compare0Top = true; // reset counter when counter reaches compare value
      burtcInit.em4comp = false; // BURTC compare interrupt wakes from EM4 (causes reset)
      BURTC_Init (&burtcInit);
      BURTC_CounterReset ();
      BURTC_CompareSet (0, 50);
      BURTC_IntEnable (BURTC_IEN_COMP);    // compare match
      NVIC_SetPriority (BURTC_IRQn, 4);
      NVIC_EnableIRQ (BURTC_IRQn);
      BURTC_Enable (true);
    }
 // sl_led_turn_off (&sl_led_comms);
}


/*!       BURTC_IRQHandler :: ISR
   @brief BURTC interrupt handler, used for sensor timeout masking

   @param void

   @return void
*/
void BURTC_IRQHandler(void)
{
  if (!GPIO_PinInGet (gpioPortC, 5))
    {
      sl_led_turn_on (&sl_led_comms);
      sl_led_turn_on (&sl_led_stat);
      selfInfo.trigd = 0;
      selfInfo.state = S_ACTIVE;
      endtx = true;
      GPIO_EM4EnablePinWakeup (GPIO_IEN_EM4WUIEN7,
                               1 << _GPIO_IEN_EM4WUIEN7_SHIFT);
  }
  else{
      GPIO_EM4EnablePinWakeup(GPIO_IEN_EM4WUIEN7, 0 << _GPIO_IEN_EM4WUIEN7_SHIFT);
  }
  BURTC_CounterReset ();
  NVIC_DisableIRQ (BURTC_IRQn);
  BURTC_Enable (false);
  BURTC_IntClear (BURTC_IF_COMP);
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
      if(selfInfo.state == S_INACTIVE || selfInfo.state == S_FAULT_OPN){
      GPIO_PinModeSet (gpioPortC, 5, gpioModeInput, 1);
      if(GPIO_PinInGet(gpioPortC, 5) == 1)
        selfInfo.state = S_FAULT_OPN;
      else
        selfInfo.state = S_INACTIVE;
      GPIO_PinModeSet (gpioPortC, 5, gpioModeDisabled, 1);
      }
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
      initSensorInfo (&selfInfo, HW_ACSN, S_INACTIVE, 0, emberGetNodeType(),
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

