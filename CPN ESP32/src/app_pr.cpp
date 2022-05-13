#include <Arduino.h>
#include "app_pr.h"
#include "app_common.h"

bool prPowerChg = false;
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
            prPowerChg = true;
        }
        else if (!digitalRead(PR_PGOOD) && digitalRead(PR_CHG))
        {
            prPowerDc = true;
            prPowerChg = false;
        }
        else
        {
            prPowerDc = false;
            prPowerChg = false;
        }
        vTaskDelay(250);
    }
}
