#ifndef APP_GUI_H
#define APP_GUI_H

void displayTask(void *pvParameters);
void displayChangeAnimTask(void *pvParameters);
extern QueueHandle_t manager2GuiDeviceIndexQueue;
extern bool FLAGwifiIsConnected;
extern bool FLAGfirebaseActive;
extern uint8_t selection;
extern bool swflag;
#endif  // APP_GUI_H