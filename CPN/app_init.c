#include "app_log.h"
#include "sl_app_common.h"
#include "stack/include/ember.h"
#include "app_process.h"
#include "app_framework_common.h"
#include "sl_simple_led_instances.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_eusart.h"

EmberKeyData security_key = { .contents = { █████████
    █████████
    █████████ } };

void emberAfInitCallback(void)
{
  EmberStatus status;
  status = 0x01;

  while(status != (EMBER_SUCCESS || EMBER_NOT_JOINED))
    {
      status = emberNetworkInit();
      sl_sleeptimer_delay_millisecond(500);
      app_log_info("init Network status 0x%02X\n", status);
    }
  status = 0x01;
  emberResetNetworkState ();
}
