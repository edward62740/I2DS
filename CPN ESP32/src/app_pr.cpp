#include <Arduino.h>
#include "app_pr.h"
#include "app_common.h"

bool prPowerFail = false;
bool prPowerDc = true;

void powerReserve(void *pvParameters)
{
    pinMode(PR_PGOOD, INPUT);
    pinMode(PR_CHG, INPUT);
    while (1)
    {
        if (!digitalRead(PR_PGOOD) && !digitalRead(PR_CHG))
        {
            prPowerDc = true;
            prPowerFail = false;
        }
        else if (!digitalRead(PR_PGOOD) && digitalRead(PR_CHG))
        {
            prPowerDc = true;
            prPowerFail = false;
        }
        else if (digitalRead(PR_PGOOD))
        {
            prPowerDc = false;
            prPowerFail = true;
        }
        vTaskDelay(250);
    }
}
