#ifndef APP_RADIO_H
#define APP_RADIO_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

EmberStatus applicationSensorRadioInit(void);
void applicationSensorTxInit(void);
void applicationSensorTxStartEvent(void);
void applicationSensorTxEndEvent(void);
void applicationSensorTxRoutine(void);
void applicationSensorTxReply(bool success);
void applicationSensorRxMsg(EmberIncomingMessage *message);

#endif  // APP_RADIO_H
