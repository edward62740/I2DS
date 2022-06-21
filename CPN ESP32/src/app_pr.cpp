#include <Arduino.h>
#include "app_pr.h"
#include "app_common.h"
#include "app_manager.h"

bool prPowerChg = false;
bool prPowerDc = true;

/*! powerReserve()
   @brief monitors bq24072 battery status pins, raises flags for dc power present and charging. Results interpreted by GUI/Firebase task

   @note 

   @param void
*/
void prTask(void *pvParameters)
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
            last_reset_reason = RST_POR;
            prPowerDc = false;
            prPowerChg = false;
        }
        vTaskDelay(250);
    }
}
