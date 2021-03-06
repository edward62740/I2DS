#include "app_init.h"
#include "app_log.h"
#include "sl_app_common.h"
#include "sl_sleeptimer.h"
#include "app_process.h"
#include "app_framework_common.h"
#include "em_iadc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_prs.h"
#include "em_gpio.h"
#include "em_lesense.h"
#include "em_acmp.h"
#include "em_pcnt.h"
#include "sl_simple_led_instances.h"

volatile bool firsttrig = true;
volatile bool coldstart = true;
EmberKeyData security_key = { .contents = { █████████
    █████████
    █████████ } };

void emberAfInitCallback(void)
{
  emberAfAllocateEvent (&report_control, &report_handler);
  sl_led_turn_on (&sl_led_stat);
  EmberStatus status;
  while (status != EMBER_SUCCESS)
    {
      sl_sleeptimer_delay_millisecond (500);
      status = emberNetworkInit ();
    }

  sl_sleeptimer_delay_millisecond (100);
  app_log_info("Network status 0x%02X\n", status);

  while (emberSetSecurityKey (&security_key) != EMBER_SUCCESS)
    ;
  app_log_info("Network status 0x%02X\n", status);
  EmberNetworkParameters parameters;
  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  parameters.radioTxPower = 100;
  parameters.radioChannel = 11;
  parameters.panId = 0x01FF;
  status = emberJoinNetwork(EMBER_STAR_SLEEPY_END_DEVICE, &parameters);
  app_log_info("Network status 0x%02X\n", status);

 emberAfPluginPollEnableShortPolling (true);
 CMU_ClockSelectSet (cmuClock_EM4GRPACLK, cmuSelect_ULFRCO);
 CMU_ClockEnable (cmuClock_BURTC, true);
 CMU_ClockEnable (cmuClock_BURAM, true);
 BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
 burtcInit.compare0Top = true; // reset counter when counter reaches compare value
 BURTC_Init (&burtcInit);
 BURTC_CompareSet (0, 15000);
 BURTC_IntEnable (BURTC_IEN_COMP);    // compare match
 NVIC_SetPriority (BURTC_IRQn, 4);
 NVIC_EnableIRQ (BURTC_IRQn);
 BURTC_Enable (true);
 coldstart = true;
}

/*!       startBatteryMonitor :: FUNCTION
   @brief enable and start supply voltage readings on prs radio trigger

   @param void

   @return void
*/
void startBatteryMonitor(void)
{
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t singleInput = IADC_SINGLEINPUT_DEFAULT;

  CMU_ClockEnable (cmuClock_PRS, true);
  PRS_SourceAsyncSignalSet (0,
  PRS_ASYNC_CH_CTRL_SOURCESEL_MODEM,
                            PRS_MODEMH_PRESENT);
  PRS_ConnectConsumer (0, prsTypeAsync, prsConsumerIADC0_SINGLETRIGGER);
  CMU_ClockEnable (cmuClock_IADC0, true);
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = 1200;
  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed2x;

  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale (
      IADC0, 1000000, 0, iadcCfgModeNormal, init.srcClkPrescale);
  initSingle.triggerSelect = iadcTriggerSelPrs0PosEdge;
  initSingle.dataValidLevel = iadcFifoCfgDvl4;
  initSingle.start = true;
  singleInput.posInput = iadcPosInputAvdd;
  singleInput.negInput = iadcNegInputGnd;
  IADC_init (IADC0, &init, &initAllConfigs);
  IADC_initSingle (IADC0, &initSingle, &singleInput);
  IADC_clearInt (IADC0, _IADC_IF_MASK);
  IADC_enableInt (IADC0, IADC_IEN_SINGLEDONE);
  NVIC_ClearPendingIRQ (IADC_IRQn);
  NVIC_SetPriority(GPIO_ODD_IRQn, 7);
  NVIC_EnableIRQ (IADC_IRQn);
}

/*!       startSensorMonitor :: FUNCTION
   @brief enable and start sensor gpio interrupt

   @param void

   @return void
*/

bool startSensorMonitor(void)
{
  if(coldstart) {
      selfInfo.state = S_COLDSTART;
      return false;
  }
  firsttrig = true;
  selfInfo.state = S_ACTIVE;
  CMU_ClockEnable (cmuClock_GPIO, true);
  GPIO_PinModeSet(gpioPortC, 5, gpioModeInput, 1);

  GPIO_EM4EnablePinWakeup(GPIO_IEN_EM4WUIEN7, 1 << _GPIO_IEN_EM4WUIEN7_SHIFT);
  GPIO->IEN = 1 << _GPIO_IEN_EM4WUIEN7_SHIFT;
  GPIO->EM4WUEN = 1 << _GPIO_IEN_EM4WUIEN7_SHIFT;

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_SetPriority(GPIO_ODD_IRQn, 5);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  return true;
}

/*!       endSensorMonitor :: FUNCTION
   @brief disable and end sensor gpio interrupt

   @param void

   @return void
*/
void endSensorMonitor(void)
{
  firsttrig = false;
  selfInfo.state = S_INACTIVE;
  NVIC_ClearPendingIRQ (GPIO_ODD_IRQn);
  NVIC_DisableIRQ (GPIO_ODD_IRQn);
  GPIO_EM4DisablePinWakeup (GPIO_IEN_EM4WUIEN7);
  GPIO->IEN = 0 << _GPIO_IEN_EM4WUIEN7_SHIFT;
  GPIO->EM4WUEN = 0 << _GPIO_IEN_EM4WUIEN7_SHIFT;
  GPIO_PinModeSet (gpioPortC, 5, gpioModeDisabled, 1);
}
