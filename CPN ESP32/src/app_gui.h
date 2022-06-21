#ifndef APP_GUI_H
#define APP_GUI_H

void displayTask(void *pvParameters);
void displayChangeAnimTask(void *pvParameters);


extern QueueHandle_t manager2GuiDeviceIndexQueue;

extern bool FLAGwifiIsConnected;
extern bool FLAGfirebaseActive;
extern uint8_t guiDeviceSelectedIndex;
extern bool guiDeviceSelectedSw;
#endif  // APP_GUI_H