#ifndef APP_IPC_H
#define APP_IPC_H

void ipcTask(void *pvParameters);
bool ipcSender(uint16_t id, uint8_t state);
bool ipcParser(char *buffer, size_t len);


typedef enum
{                    /* Sensor state byte */
  S_ACTIVE = 0x05,   // Sensor element active
  S_INACTIVE,        // Sensor element inactive
  S_FAULT_HW = 0xCA, // Hardware fault detected
  S_FAULT_OPN,       // Operational fault detected
  S_ALERTING,        // Sensor element triggering
  S_COLDSTART = 0xE0,
  S_CALIBRATING,
  S_WATCHING = 0xFA, // Reserved
} sensor_state_t;

typedef enum
{                /* Hardware identification byte */
  HW_CPN = 0x88, // Control Panel Node (Coordinator)
  HW_PIRSN,      // PIR Sensor Node (Sensor)
  HW_ACSN,       // Access Control Sensor Node (Sensor)
  HW_SENTINEL, 
} device_hw_t;

/* IPC Information */
typedef enum
{                   /* IPC message identification byte */
  IPC_CHANGE,       // (C -> EXT) notify change of information
  IPC_LIST,         // (C -> EXT) send list of connected sensors and associated information
  IPC_REQUEST,      // (C <- EXT) request sensor change state
  IPC_REQUEST_ACK,  // (C -> EXT) ack IPC_REQUEST
  IPC_REQUEST_DONE, // (C -> EXT) finished IPC_REQUEST
  IPC_LIST_CTS,     // (C <- EXT) ack IPC_CHANGE and request IPC_LIST
  IPC_REPORT,
  IPC_ERR,
} ipc_message_pid_t;

extern bool FLAGguiUpdatePending;
extern bool FLAGipcResponsePending;
extern uint8_t ipc_get_list[3];
extern uint8_t ipc_set_sensor_state[6];
extern TimerHandle_t ipcDeviceResponseTimer;
extern uint16_t tmpTimerId;
extern uint8_t tmpTimerState;
extern TimerHandle_t managerDeviceTimer[30];
#endif  // APP_IPC_H