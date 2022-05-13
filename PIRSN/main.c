#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "sl_power_manager.h"
#include "app_init.h"
#include "app_process.h"
#include "sl_system_process_action.h"



int main(void)
{
  sl_system_init();
  while (1) {
    sl_system_process_action();
    sl_power_manager_sleep();
  }
}

