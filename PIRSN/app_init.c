/***************************************************************************//**
 * @file
 * @brief app_init.c
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
#include "app_log.h"
#include "sl_app_common.h"
#include "sl_si70xx.h"
#include "sl_i2cspm_instances.h"
#include "sl_sleeptimer.h"
#include "app_process.h"
#include "app_framework_common.h"
#include "em_iadc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_prs.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/******************************************************************************
* Application framework init callback
******************************************************************************/
void emberAfInitCallback(void)
{
  EmberStatus status;

  emberAfAllocateEvent(&report_control, &report_handler);
  // CLI info message
  app_log_info("\nSensor\n");

  status = emberNetworkInit();
  app_log_info("Network status 0x%02X\n", status);
  emberResetNetworkState();
  status = emberSetSecurityKey(&security_key);
  app_log_info("Network status 0x%02X\n", status);

  EmberNetworkParameters parameters;
MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
parameters.radioTxPower = 0;
parameters.radioChannel = 11;
parameters.panId = 0x01FF;
status = emberJoinNetwork(EMBER_STAR_SLEEPY_END_DEVICE, &parameters);
app_log_info("Network status 0x%02X\n", status);
#if defined(EMBER_AF_PLUGIN_BLE)
  bleConnectionInfoTableInit();
#endif
  emberAfPluginPollEnableShortPolling(true);

  IADC_Init_t init = IADC_INIT_DEFAULT;
   IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
   IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;

   // Single input structure
   IADC_SingleInput_t singleInput = IADC_SINGLEINPUT_DEFAULT;

   /*
    * Enable IADC0 and GPIO register clock.
    *
    * Note: On EFR32xG21 devices, CMU_ClockEnable() calls have no effect
    * as clocks are enabled/disabled on-demand in response to peripheral
    * requests.  Deleting such lines is safe on xG21 devices and will
    * reduce provide a small reduction in code size.
    */

   CMU_ClockEnable(cmuClock_PRS, true);

    // Connect the specified PRS channel to the LETIMER producer
    PRS_SourceAsyncSignalSet(0,
                             PRS_ASYNC_CH_CTRL_SOURCESEL_MODEM,
                             PRS_MODEMH_PRESENT);

    // Connect the specified PRS channel to the IADC as the consumer
    PRS_ConnectConsumer(0,
                        prsTypeAsync,
                        prsConsumerIADC0_SINGLETRIGGER);

   CMU_ClockEnable(cmuClock_IADC0, true);




   initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
   initAllConfigs.configs[0].vRef = 1200;
   initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed2x;

   initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                      1000000,
                                                                      0,
                                                                      iadcCfgModeNormal,
                                                                      init.srcClkPrescale);


   initSingle.triggerSelect = iadcTriggerSelPrs0PosEdge;
   initSingle.dataValidLevel = iadcFifoCfgDvl4;
   initSingle.start = true;


   singleInput.posInput   = iadcPosInputAvdd;
   singleInput.negInput   = iadcNegInputGnd;

   // Initialize IADC
   IADC_init(IADC0, &init, &initAllConfigs);
   // Initialize a single-channel conversion
   IADC_initSingle(IADC0, &initSingle, &singleInput);

   // Clear any previous interrupt flags
   IADC_clearInt(IADC0, _IADC_IF_MASK);

   // Enable single-channel done interrupts
   IADC_enableInt(IADC0, IADC_IEN_SINGLEDONE);

   // Enable IADC interrupts
   NVIC_ClearPendingIRQ(IADC_IRQn);
   NVIC_EnableIRQ(IADC_IRQn);


  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
       sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM2);
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------