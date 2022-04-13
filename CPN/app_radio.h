#ifndef APP_RADIO_H
#define APP_RADIO_H

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
#include "app_ipc.h"


bool applicationCoordinatorTxIdentify(EmberNodeId id);
bool applicationCoordinatorTxRequest(EmberNodeId id, message_request_t type, uint8_t val);
void applicationCoordinatorRxMsg(EmberIncomingMessage *message);


#endif  // APP_RADIO_H
