#ifndef APP_INIT_H
#define APP_INIT_H

#include PLATFORM_HEADER

void startBatteryMonitor(void);
bool startSensorMonitor(void);
void endSensorMonitor(void);
extern volatile bool firsttrig;
extern volatile bool coldstart;
#endif  // APP_INIT_H
