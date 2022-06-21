#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include "TFT_eSPI.h"
#include <XPT2046.h>
#include "app_common.h"
#include "app_pr.h"

// Timer for updating GUI head
TimerHandle_t guiHeadUpdateTimer;
// Timer for debouncing touch screen activation
TimerHandle_t guiTouchDebounceTimer;
// Timer for switching screen to sleep mode
TimerHandle_t guiNoTouchTimer;

// Queue for receiving GUI updates from manager
QueueHandle_t manager2GuiDeviceIndexQueue;

SemaphoreHandle_t xGuiHeadUpdate;
SemaphoreHandle_t xGuiTouchReady;

bool guiIsAlerting = false;
bool guiClean = false;

// Initialization flags
bool FLAGwifiIsConnected = false;
bool FLAGfirebaseActive = false;

// Indication when touched
bool guiDeviceSelectedSw = false;
uint8_t guiDeviceSelectedIndex = 0;
// Sleep screen
bool guiIsSleeping = false;

XPT2046 touch(16, 17);
TFT_eSPI tft = TFT_eSPI();

/*! guiHeadUpdateTimerCallback()
   @brief callback for GUI header update

   @note

   @param guiHeadUpdateTimer FreeRTOS timer handle
*/
void guiHeadUpdateTimerCallback(TimerHandle_t guiHeadUpdateTimer)
{
    xSemaphoreGiveFromISR(xGuiHeadUpdate, NULL);
}

/*! guiHeadUpdateTimerCallback()
   @brief callback for GUI header update

   @note raises flag

   @param guiHeadUpdateTimer FreeRTOS timer handle
*/
void guiNoTouchTimerCallback(TimerHandle_t guiNoTouchTimer)
{
    guiIsSleeping = true;
    ledcWrite(15, 2);
    APP_LOG_INFO("SLEEP");
    xTimerStop(guiNoTouchTimer, 0);
}

/*! guiHeadUpdateTimerCallback()
   @brief callback for GUI touch debounce

   @note gives xGuiTouchReady

   @param guiTouchDebounceTimer FreeRTOS timer handle
*/
void guiTouchDebounceTimerCallback(TimerHandle_t guiTouchDebounceTimer)
{
    xSemaphoreGiveFromISR(xGuiTouchReady, NULL);
    APP_LOG_INFO("EXIT");
    xTimerStop(guiTouchDebounceTimer, 0);
}

/*! displayTask()
   @brief task to update the GUI when touched, or receives updates passed from manager2GuiDeviceIndexQueue.

   @note

   @param void
*/
void displayTask(void *pvParameters)
{
    xGuiHeadUpdate = xSemaphoreCreateBinary();
    xGuiTouchReady = xSemaphoreCreateBinary();
    guiHeadUpdateTimer = xTimerCreate("guiHead", GUI_HEAD_UPDATE_INTERVAL_MS, pdTRUE, (void *)0, guiHeadUpdateTimerCallback);
    guiNoTouchTimer = xTimerCreate("guiSleep", GUI_SLEEP_IF_NO_TOUCH_MS, pdTRUE, (void *)0, guiNoTouchTimerCallback);
    guiTouchDebounceTimer = xTimerCreate("guiTouch", GUI_TOUCH_DEBOUNCE_MS, pdTRUE, (void *)0, guiTouchDebounceTimerCallback);
    touch.begin(240, 320); // Must be done before setting rotation
    tft.begin();
    pinMode(STAT_LED, OUTPUT);
    ledcSetup(15, 5000, 8);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(TFT_LED, 15);
    ledcWrite(15, 255);
    tft.setTextWrap(true, true);
    tft.setTextSize(7);
    tft.print("I2DS");
    tft.setTextSize(4);
    tft.println("CPN");
    tft.setCursor(0, 60);
    tft.setTextSize(1);
    tft.println("boot..");
    vTaskDelay(50);
    tft.println("Initializing I2DS Control Panel Node");
    vTaskDelay(50);
    tft.println("Starting Manager");
    vTaskDelay(50);
    tft.println("Initializing IPC");
    vTaskDelay(50);
    tft.println("/tbd SE/");
    vTaskDelay(50);
    tft.println("Starting Power Reserve Subsystem");
    vTaskDelay(50);
    tft.println("Allocating stack");
    vTaskDelay(50);
    tft.println("Connecting to Wi-Fi...");

    while (!FLAGwifiIsConnected)
    {
        vTaskDelay(100);
        digitalWrite(STAT_LED, !digitalRead(STAT_LED));
    }
    tft.println("Authenticating Firebase Database...");
    while (!FLAGfirebaseActive)
    {
        vTaskDelay(100);
        digitalWrite(STAT_LED, !digitalRead(STAT_LED));
    }
    tft.fillScreen(ILI9341_BLACK);
    touch.setRotation(touch.ROT180);
    touch.setCalibration(209, 1759, 1775, 273);

    selfInfoExt.touchArea[0] = 0;
    selfInfoExt.touchArea[1] = 0;
    selfInfoExt.touchArea[2] = 240;
    selfInfoExt.touchArea[3] = 54;
    selfInfo.state = (uint8_t)S_INACTIVE;
    xTimerStart(guiHeadUpdateTimer, 0);
    xTimerStart(guiNoTouchTimer, 0);
    xSemaphoreGive(xGuiTouchReady);

    while (1)
    {
        if (!guiIsAlerting && (xSemaphoreTake(xGuiHeadUpdate, 0) == pdTRUE))
        {
            uint8_t tmp = 0;
            for (uint8_t i = 0; i < sensorIndex; i++)
            {
                if (sensorInfo[i].state == (uint8_t)S_ACTIVE)
                    tmp++;
            }
            if (tmp > (sensorIndex / 2))
                selfInfo.state = (uint8_t)S_ACTIVE;
            else
                selfInfo.state = (uint8_t)S_INACTIVE;
            digitalWrite(STAT_LED, !digitalRead(STAT_LED));
            tft.setFreeFont(&FreeSansBold12pt7b);
            tft.setTextDatum(MC_DATUM);
            if (guiClean)
            {
                tft.fillScreen(ILI9341_BLACK);
                guiClean = false;
                APP_LOG_INFO("CLEARED");
            }

            tft.fillRect(0, 0, 240, 50, ILI9341_RED);
            // tft.fillSmoothRoundRect(2, 2, 236, 50, 5, ILI9341_ORANGE);
            // tft.drawFastHLine(5, 25, 230, ILI9341_BLACK);

            if (selfInfo.state == (uint8_t)S_ACTIVE)
            {
                tft.setTextColor(ILI9341_BLUE, ILI9341_RED, true);
                tft.setCursor(11, 23);
                tft.print("I2DS Control Panel");
                tft.setCursor(9, 21);
                tft.print("I2DS Control Panel");
                tft.setCursor(11, 21);
                tft.print("I2DS Control Panel");
                tft.setCursor(9, 23);
                tft.print("I2DS Control Panel");
                tft.setCursor(10, 22);
                tft.setTextColor(ILI9341_BLACK, ILI9341_BLUE, true);
            }
            else if (selfInfo.state == (uint8_t)S_INACTIVE)
            {
                tft.setTextColor(ILI9341_BLACK, ILI9341_RED, true);
                tft.setCursor(11, 23);
                tft.print("I2DS Control Panel");
                tft.setCursor(9, 21);
                tft.print("I2DS Control Panel");
                tft.setCursor(11, 21);
                tft.print("I2DS Control Panel");
                tft.setCursor(9, 23);
                tft.print("I2DS Control Panel");
                tft.setCursor(10, 22);
                tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK, true);
            }

            tft.print("I2DS Control Panel");
            tft.setTextFont(1);
            tft.setTextColor(ILI9341_GREEN, ILI9341_RED, true);
            tft.setCursor(10, 30);
            tft.print("PRS: ");
            tft.setCursor(35, 30);
            if (!prPowerChg && prPowerDc)
            {
                tft.setTextColor(ILI9341_GREEN, ILI9341_RED, true);
                tft.print("CONNECTED TO USB POWER");
            }
            else if (prPowerChg && prPowerDc)
            {
                tft.setTextColor(ILI9341_ORANGE, ILI9341_RED, true);
                tft.print("CHARGING BATTERY RESERVES");
            }
            else
            {
                tft.setTextColor(ILI9341_MAROON, ILI9341_RED, true);
                tft.print("USB POWER FAILURE");
            }
            tft.setTextColor(ILI9341_WHITE, ILI9341_RED, true);
            tft.setCursor(10, 40);
            tft.print("Devices: ");
            tft.setCursor(0, 310);
            tft.print("FW " + (String)FW_VERSION);
            if (sensorIndex > GUI_MAX_ITEMS)
            {
                tft.setCursor(67, 290);
                tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK, true);
                tft.print("VIEW IN APP");
                tft.fillTriangle(80, 300, 120, 300, 100, 320, ILI9341_WHITE);
            }
            tft.setTextColor(ILI9341_WHITE, ILI9341_RED, true);
            tft.setCursor(58, 40);
            tft.print((int)sensorIndex);
            uint8_t tmpcount = 0;
            for (uint8_t i = 0; i < sensorIndex; i++)
            {
                if (sensorInfoExt[i].alive)
                    tmpcount++;
            }
            tft.setCursor(78, 40);
            tft.print("Connected: ");
            tft.setCursor(138, 40);
            tft.print((int)tmpcount);
            tft.setCursor(160, 40);
            tft.fillRoundRect(200, 290, 40, 30, 3, ILI9341_BLUE);
            tft.setSwapBytes(true);
            tft.pushImage(210, 295, 20, 20, reload);
            tft.setSwapBytes(true);
            // tft.pushImage();
            if (FLAGwifiIsConnected)
                tft.pushImage(210, 26, 30, 22, wifilogo);
            tft.setCursor(214, 32);
        }

        if (FLAGipcResponsePending || guiDeviceSelectedSw || guiIsAlerting)
        {
            if (guiFlashOnAlert && guiIsAlerting) // if GUI flash on alert is enabled from app
            {
                if (guiIsSleeping)
                {
                    xTimerStart(guiNoTouchTimer, 0);
                    guiIsSleeping = false;
                    for (uint8_t i = 2; i < 255; i++)
                    {
                        ledcWrite(15, i);
                        vTaskDelay(1);
                    }
                }
                else
                    xTimerReset(guiNoTouchTimer, 0);
                tft.fillScreen(guiDeviceSelectedSw ? ILI9341_BLUE : ILI9341_RED);
                vTaskDelay(100);
            }
            else // blink in-progress device tile
            {
                uint16_t color;
                switch (sensorInfo[guiDeviceSelectedIndex].state)
                {
                case S_INACTIVE:
                    color = ILI9341_GREEN;
                    break;
                case S_ACTIVE:
                    color = ILI9341_DARKGREY;
                    break;
                default:
                    color = ILI9341_MAGENTA;
                    break;
                }
                tft.drawRoundRect(sensorInfoExt[guiDeviceSelectedIndex].touchArea[0] - 1, sensorInfoExt[guiDeviceSelectedIndex].touchArea[1] - 1, sensorInfoExt[guiDeviceSelectedIndex].touchArea[2] + 2, sensorInfoExt[guiDeviceSelectedIndex].touchArea[3] + 2, 5, guiDeviceSelectedSw ? ILI9341_BLACK : color);
            }
            digitalWrite(STAT_LED, !digitalRead(STAT_LED));
            guiDeviceSelectedSw = !guiDeviceSelectedSw;
            vTaskDelay(50);
        }

        if (guiIsSleeping && touch.isTouching())
        {
            xTimerStart(guiNoTouchTimer, 0);
            guiIsSleeping = false;
            for (uint8_t i = 2; i < 255; i++)
            {
                ledcWrite(15, i);
                vTaskDelay(1);
            }
            APP_LOG_INFO("WAKE");
        }
        else if (!guiIsSleeping && touch.isTouching() && !FLAGipcResponsePending && (xSemaphoreTake(xGuiTouchReady, 0) == pdTRUE)) // touch response if no ipc operation pending
        {
            xTimerStart(guiTouchDebounceTimer, 0);
            xTimerReset(guiNoTouchTimer, 0);
            uint16_t x, y;
            touch.getPosition(x, y);

            // if head touched; activate/inactivate all
            if ((int16_t)x > selfInfoExt.touchArea[0] && (int16_t)x < (selfInfoExt.touchArea[0] + selfInfoExt.touchArea[2]) &&
                (int16_t)y > selfInfoExt.touchArea[1] && (int16_t)y < (selfInfoExt.touchArea[1] + selfInfoExt.touchArea[3]))
            {
                for (uint8_t i = 0; i < sensorIndex; i++)
                {
                    if (sensorInfo[i].state == (uint8_t)S_ACTIVE || sensorInfo[i].state == (uint8_t)S_INACTIVE)
                    {
                        FirebaseReq_t tmpReq;
                        tmpReq.id = sensorInfo[i].self_id;
                        if (selfInfo.state == (uint8_t)S_INACTIVE)
                            tmpReq.state = (uint8_t)S_ACTIVE;
                        else if (selfInfo.state == (uint8_t)S_ACTIVE)
                            tmpReq.state = (uint8_t)S_INACTIVE;
                        tmpReq.index = i;
                        if (uxQueueSpacesAvailable(app2ManagerDeviceReqQueue) == 0)
                        {
                            xQueueReset(app2ManagerDeviceReqQueue);
                            err_count.MANAGER_QUEUE_REQ_OVERFLOW++;
                        }
                        if (xQueueSend(app2ManagerDeviceReqQueue, (void *)&tmpReq, 0) == 0)
                            err_count.MANAGER_QUEUE_REQ_FAIL++;
                    }
                    else
                    {
                        tft.drawRoundRect(sensorInfoExt[i].touchArea[0] - 1, sensorInfoExt[i].touchArea[1] - 1, sensorInfoExt[i].touchArea[2] + 2, sensorInfoExt[i].touchArea[3] + 2, 5, ILI9341_RED);
                        tft.drawWideLine(sensorInfoExt[i].touchArea[0] + 10, sensorInfoExt[i].touchArea[1] + 10, sensorInfoExt[i].touchArea[0] + 100, sensorInfoExt[i].touchArea[1] + 60, 2, ILI9341_RED);
                        tft.drawWideLine(sensorInfoExt[i].touchArea[0] + 100, sensorInfoExt[i].touchArea[1] + 10, sensorInfoExt[i].touchArea[0] + 10, sensorInfoExt[i].touchArea[1] + 60, 2, ILI9341_RED);
                    }
                }
            }
            else if (((int16_t)x > 200 && (int16_t)x < 240) && (int16_t)y > 290 && (int16_t)y < 320) // if gui restart touched
            {
                tft.begin();
                tft.setTextSize(2);
                tft.setTextColor(ILI9341_BLUE);
                tft.setCursor(0, 160);
                tft.print("Restarting GUI service...");
                tft.setTextSize(1);
                guiClean = true;
            }
            else // if device tile touched
            {
                for (uint8_t i = 0; i < sensorIndex; i++)
                {
                    if ((int16_t)x > sensorInfoExt[i].touchArea[0] && (int16_t)x < (sensorInfoExt[i].touchArea[0] + sensorInfoExt[i].touchArea[2]) &&
                        (int16_t)y > sensorInfoExt[i].touchArea[1] && (int16_t)y < (sensorInfoExt[i].touchArea[1] + sensorInfoExt[i].touchArea[3]))
                    {
                        if (sensorInfo[i].state == (uint8_t)S_ALERTING)
                        {

                            tft.drawWideLine(sensorInfoExt[i].touchArea[0] + 10, sensorInfoExt[i].touchArea[1] + 10, sensorInfoExt[i].touchArea[0] + 100, sensorInfoExt[i].touchArea[1] + 60, 4, ILI9341_MAGENTA);
                            tft.drawWideLine(sensorInfoExt[i].touchArea[0] + 100, sensorInfoExt[i].touchArea[1] + 10, sensorInfoExt[i].touchArea[0] + 10, sensorInfoExt[i].touchArea[1] + 60, 4, ILI9341_MAGENTA);
                        }
                        else if (sensorInfo[i].state == (uint8_t)S_ACTIVE || sensorInfo[i].state == (uint8_t)S_INACTIVE)
                        {
                            tft.drawRoundRect(sensorInfoExt[i].touchArea[0] - 1, sensorInfoExt[i].touchArea[1] - 1, sensorInfoExt[i].touchArea[2] + 2, sensorInfoExt[i].touchArea[3] + 2, 5, ILI9341_YELLOW);
                            FirebaseReq_t tmpReq;
                            tmpReq.id = sensorInfo[i].self_id;
                            if (sensorInfo[i].state == (uint8_t)S_INACTIVE)
                                tmpReq.state = (uint8_t)S_ACTIVE;
                            else if (sensorInfo[i].state == (uint8_t)S_ACTIVE)
                                tmpReq.state = (uint8_t)S_INACTIVE;
                            tmpReq.index = i;
                            if (uxQueueSpacesAvailable(app2ManagerDeviceReqQueue) == 0)
                            {
                                xQueueReset(app2ManagerDeviceReqQueue);
                                err_count.MANAGER_QUEUE_REQ_OVERFLOW++;
                            }
                            if (xQueueSend(app2ManagerDeviceReqQueue, (void *)&tmpReq, 0) == 0)
                                err_count.MANAGER_QUEUE_REQ_FAIL++;
                        }
                        else
                        {
                            tft.drawRoundRect(sensorInfoExt[i].touchArea[0] - 1, sensorInfoExt[i].touchArea[1] - 1, sensorInfoExt[i].touchArea[2] + 2, sensorInfoExt[i].touchArea[3] + 2, 5, ILI9341_RED);
                            tft.drawWideLine(sensorInfoExt[i].touchArea[0] + 10, sensorInfoExt[i].touchArea[1] + 10, sensorInfoExt[i].touchArea[0] + 100, sensorInfoExt[i].touchArea[1] + 60, 2, ILI9341_RED);
                            tft.drawWideLine(sensorInfoExt[i].touchArea[0] + 100, sensorInfoExt[i].touchArea[1] + 10, sensorInfoExt[i].touchArea[0] + 10, sensorInfoExt[i].touchArea[1] + 60, 2, ILI9341_RED);
                        }
                    }
                }
            }
        }

        // handle queue updates from manager task, to update device tiles
        if ((uxQueueMessagesWaiting(manager2GuiDeviceIndexQueue) > 0) || !guiClean)
        {
            digitalWrite(STAT_LED, !digitalRead(STAT_LED));
            uint8_t i = 0;
            uint8_t cnt = 0;
            for (uint8_t n = 0; n < sensorIndex; n++)
            {
                if (sensorInfo[n].state == (uint8_t)S_ALERTING)
                {
                    guiIsAlerting = true;
                    guiClean = true;
                    break;
                }
                else
                    cnt++;
            }
            if (cnt == sensorIndex)
            {
                guiIsAlerting = false;
            }
            if (xQueueReceive(manager2GuiDeviceIndexQueue, (void *)&i, 1) == pdPASS)
            {
                if (i + 1 <= GUI_MAX_ITEMS)
                {
                    uint16_t xsp = ((i % 2) * GUI_PANEL_REL_X_SPACING);
                    uint16_t ysp = ((i / 2) * GUI_PANEL_REL_Y_SPACING);
                    sensorInfoExt[i].touchArea[0] = GUI_PANEL_START_X_OFFSET + xsp;
                    sensorInfoExt[i].touchArea[1] = GUI_PANEL_START_Y_OFFSET + ysp;
                    sensorInfoExt[i].touchArea[2] = GUI_PANEL_BOX_WIDTH;
                    sensorInfoExt[i].touchArea[3] = GUI_PANEL_BOX_HEIGHT;
                    tft.drawRoundRect(sensorInfoExt[i].touchArea[0] - 1, sensorInfoExt[i].touchArea[1] - 1, sensorInfoExt[i].touchArea[2] + 2, sensorInfoExt[i].touchArea[3] + 2, 5, ILI9341_BLACK);
                    uint16_t bg, head;
                    switch (sensorInfo[i].state)
                    {
                    case S_INACTIVE:
                        head = ILI9341_LIGHTGREY;
                        bg = ILI9341_DARKGREY;
                        break;
                    case S_ALERTING:
                        head = ILI9341_BLUE;
                        bg = ILI9341_RED;
                        break;
                    case S_COLDSTART:
                        head = ILI9341_RED;
                        bg = ILI9341_ORANGE;
                        break;
                    case S_FAULT_OPN:
                        head = ILI9341_YELLOW;
                        bg = ILI9341_MAGENTA;
                        break;
                    default:
                        head = ILI9341_BLUE;
                        bg = ILI9341_DARKGREEN;
                        break;
                    }
                    if (!sensorInfoExt[i].alive)
                    {
                        head = ILI9341_LIGHTGREY;
                        bg = 0x000F;
                    }
                    tft.fillSmoothRoundRect(GUI_PANEL_START_X_OFFSET + xsp, GUI_PANEL_START_Y_OFFSET + ysp, GUI_PANEL_BOX_WIDTH, GUI_PANEL_BOX_HEIGHT, GUI_PANEL_BOX_CORNER_RADIUS, bg);
                    tft.fillRectVGradient(GUI_PANEL_START_X_OFFSET + xsp, GUI_PANEL_START_Y_OFFSET + ysp, GUI_PANEL_BOX_WIDTH, 32, ILI9341_BLACK, bg);
                    tft.setFreeFont(&FreeSansBold9pt7b);
                    tft.setTextColor(head, bg, true);
                    tft.setCursor(5 + xsp, 80 + ysp);
                    switch (sensorInfo[i].hw)
                    {
                    case HW_PIRSN:
                        tft.print("PIRSN");
                        break;
                    case HW_ACSN:
                        tft.print("ACSN");
                        break;
                    default:
                        tft.print("?");
                        break;
                    }
                    tft.setTextFont(2);
                    tft.setCursor(100 + xsp, 65 + ysp);
                    tft.setTextColor(ILI9341_WHITE);
                    tft.print(sensorInfo[i].self_id);
                    tft.setTextColor(ILI9341_WHITE, bg, true);
                    tft.setCursor(5 + xsp, 90 + ysp);
                    if (sensorInfoExt[i].alive)
                        switch (sensorInfo[i].state)
                        {
                        case S_ACTIVE:
                            tft.print("ACTIVE");
                            break;
                        case S_ALERTING:
                            tft.print("ALERTING");
                            break;
                        case S_COLDSTART:
                            tft.print("WARMUP");
                            break;
                        case S_INACTIVE:
                            (sensorInfo[i].hw == HW_ACSN) ? tft.print("INACTIVE-CLOSED") : tft.print("INACTIVE");
                            break;
                        case S_FAULT_HW:
                            tft.print("HW FAULT");
                            break;
                        case S_FAULT_OPN:
                            (sensorInfo[i].hw == HW_ACSN) ? tft.print("INACTIVE - OPEN") : tft.print("INVALID");
                            break;
                        default:
                            break;
                        }
                    else
                        tft.print("LOST SIGNAL");
                    tft.setTextColor(ILI9341_WHITE, bg, true);
                    tft.drawRect(5 + xsp, 110 + ysp, 30, 15, ILI9341_BLACK);
                    (sensorInfoExt[i].alive) ? tft.fillRect(5 + xsp, 110 + ysp, (int16_t)(sensorInfo[i].lqi / 8), 15, ILI9341_BLACK)
                                             : tft.drawRect(5 + xsp, 110 + ysp, (int16_t)(sensorInfo[i].lqi / 8), 15, 0x6B6D);
                    tft.fillTriangle(5 + xsp, 110 + ysp, 5 + xsp, 110 + ysp + 15, 5 + xsp + 30, 110 + ysp, bg);
                    tft.setTextFont(1);
                    tft.setCursor(5 + xsp, 110 + ysp);
                    tft.print("LQI");
                    tft.fillRect(45 + xsp, 109 + ysp, 5, 2, ILI9341_BLACK);
                    tft.drawRect(40 + xsp, 111 + ysp, 15, 14, ILI9341_BLACK);
                    for (uint32_t batt = 0; 2000 + (batt * 250) < sensorInfo[i].battery_voltage; batt++)
                    {
                        if (batt > 4)
                        {
                            tft.setTextColor(ILI9341_WHITE);
                            tft.fillRect(40 + xsp, 111 + ysp, 15, 14, ILI9341_BLACK);
                            tft.setCursor(45 + xsp, 115 + ysp);
                            tft.print("!");
                            break;
                        }
                        tft.fillRect(40 + xsp, 123 + ysp - batt * 3, 15, 2, ILI9341_BLACK);
                    }
                    tft.setTextColor(ILI9341_WHITE, bg, true);
                    tft.setSwapBytes(true);
                    tft.setCursor(60 + xsp, 110 + ysp);
                    if (sensorInfoExt[i].alive)
                    {
                        switch (sensorInfo[i].state)
                        {
                        case S_INACTIVE:
                            tft.setTextColor(ILI9341_ORANGE);
                            tft.pushImage(60 + xsp, 110 + ysp, 16, 16, loc);
                            break;
                        case S_ACTIVE:
                            tft.setTextColor(ILI9341_ORANGE);
                            tft.pushImage(60 + xsp, 110 + ysp, 16, 16, loc2);
                            break;
                        case S_ALERTING:
                            tft.setTextColor(ILI9341_BLUE);
                            tft.pushImage(60 + xsp, 110 + ysp, 16, 16, loc3);
                            break;
                        case S_FAULT_OPN:
                            tft.setTextColor(ILI9341_ORANGE);
                            tft.pushImage(60 + xsp, 110 + ysp, 16, 16, loc4);
                            break;
                        }
                        tft.print("S");
                    }
                    tft.setCursor(80 + xsp, 110 + ysp);
                    tft.print(sensorInfo[i].rssi);
                    tft.setTextColor(ILI9341_BLACK, bg, true);
                    tft.setCursor(80 + xsp, 118 + ysp);
                    tft.print("dBm");
                }
            }
        }
        vTaskDelay(1);
    }
}