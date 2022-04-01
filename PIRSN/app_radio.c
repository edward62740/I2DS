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
#include "em_burtc.h"
#include "em_acmp.h"
#include "sl_app_common.h"
#include "app_process.h"
#include "app_framework_common.h"
#include "app_process.h"
#include "sl_simple_led_instances.h"
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_component_catalog.h"
#include "sl_power_manager.h"
#endif

void applicationSensorTxInit(void)
{

}
