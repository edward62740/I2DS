#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include <XPT2046.h>
#include "app_common.h"
#include "app_pr.h"
#include "TFT_eSPI.h"

TimerHandle_t guiHeadUpdateTimer;
bool guiHeadUpdateFlag = true;
bool FLAGwifiIsConnected = false;
bool FLAGfirebaseActive = false;
extern "C"
{
    uint8_t temprature_sens_read();
}

/* Objects */
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
XPT2046 touch(16, 17);
uint8_t selection = 0;
QueueHandle_t manager2GuiDeviceIndexQueue;
TFT_eSPI tft = TFT_eSPI();
bool massActivate = false;
uint8_t massActivateIndex = 0;
bool swflag = false;
void guiHeadUpdateTimerCallback(TimerHandle_t guiHeadUpdateTimer)
{
    guiHeadUpdateFlag = true;
}
void displayTask(void *pvParameters)
{
    touch.begin(240, 320); // Must be done before setting rotation
    tft.begin();
    pinMode(STAT_LED, OUTPUT);
    tft.setTextWrap(true, true);
    tft.setTextSize(7);
    tft.println("I2DS");
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
    // Replace these for your screen module
    touch.setCalibration(209, 1759, 1775, 273);

    selfInfoExt.touchArea[0] = 0;
    selfInfoExt.touchArea[1] = 0;
    selfInfoExt.touchArea[2] = 240;
    selfInfoExt.touchArea[3] = 54;
    selfInfo.state = (uint8_t)S_INACTIVE;

    guiHeadUpdateTimer = xTimerCreate("guiHead", GUI_HEAD_UPDATE_INTERVAL_MS, pdTRUE, (void *)0, guiHeadUpdateTimerCallback);
    xTimerStart(guiHeadUpdateTimer, 0);
    while (1)
    {

        if (guiHeadUpdateFlag)
        {
            digitalWrite(STAT_LED, !digitalRead(STAT_LED));
            tft.setFreeFont(&FreeSansBold12pt7b);
            tft.setTextDatum(MC_DATUM);

            tft.fillRectHGradient(0, 0, 240, 50, ILI9341_ORANGE, ILI9341_RED);
            // tft.fillSmoothRoundRect(2, 2, 236, 50, 5, ILI9341_ORANGE);
            // tft.drawFastHLine(5, 25, 230, ILI9341_BLACK);

            if (selfInfo.state == (uint8_t)S_ACTIVE)
            {
                tft.setTextColor(ILI9341_BLUE);
                tft.setCursor(11, 23);
                tft.print("I2DS Control Panel");
                tft.setCursor(9, 21);
                tft.print("I2DS Control Panel");
                tft.setCursor(11, 21);
                tft.print("I2DS Control Panel");
                tft.setCursor(9, 23);
                tft.print("I2DS Control Panel");
                tft.setCursor(10, 22);
                tft.setTextColor(ILI9341_BLACK);
            }
            else if (selfInfo.state == (uint8_t)S_INACTIVE)
            {
                tft.setCursor(10, 22);
                tft.setTextColor(ILI9341_DARKGREY);
            }

            tft.print("I2DS Control Panel");
            tft.setTextFont(1);
            tft.setTextColor(ILI9341_RED);
            tft.setCursor(10, 30);
            tft.print("PRS: ");
            tft.setCursor(35, 30);
            if (prPowerDc)
            {
                tft.setTextColor(ILI9341_GREEN);
                tft.print("DC");
            }
            else if (!prPowerDc)
            {
                tft.print("CHG");
            }
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(10, 40);
            tft.print("Devices: ");
            tft.setCursor(58, 40);
            tft.print((int)sensorIndex);
            tft.fillRoundRect(200, 290, 40, 30, 3, ILI9341_BLUE);
            tft.setSwapBytes(true);
            tft.pushImage(210, 295, 20, 20, reload);
            tft.setSwapBytes(true);
            // tft.pushImage();
            tft.pushImage(210, 26, 30, 22, wifilogo);
            tft.setCursor(212, 30);
            tft.print((const char *)WIFI_SSID);
            guiHeadUpdateFlag = false;
        }
        if (FLAGipcResponsePending || swflag)
        {
            digitalWrite(STAT_LED, !digitalRead(STAT_LED));
            uint16_t color;
            switch (sensorInfo[selection].state)
            {
            case S_INACTIVE:
                color = ILI9341_GREEN;
                break;
            case S_ALERTING:
                color = ILI9341_MAGENTA;
                break;
            default:
                color = ILI9341_DARKGREY;
                break;
            }
            tft.drawRoundRect(sensorInfoExt[selection].touchArea[0] - 1, sensorInfoExt[selection].touchArea[1] - 1, sensorInfoExt[selection].touchArea[2] + 2, sensorInfoExt[selection].touchArea[3] + 2, 5, swflag ? ILI9341_BLACK : color);
            swflag = !swflag;
            vTaskDelay(50);
        }
        if (touch.isTouching() && !FLAGipcResponsePending)
        {
            uint16_t x, y;
            touch.getPosition(x, y);
            if ((int16_t)x > selfInfoExt.touchArea[0] && (int16_t)x < (selfInfoExt.touchArea[0] + selfInfoExt.touchArea[2]) &&
                (int16_t)y > selfInfoExt.touchArea[1] && (int16_t)y < (selfInfoExt.touchArea[1] + selfInfoExt.touchArea[3]))
            {
                massActivate = true;
            }
            else
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
                        else
                        {
                            tft.drawRoundRect(sensorInfoExt[i].touchArea[0] - 1, sensorInfoExt[i].touchArea[1] - 1, sensorInfoExt[i].touchArea[2] + 2, sensorInfoExt[i].touchArea[3] + 2, 5, ILI9341_YELLOW);
                            if (sensorInfo[i].state == (uint8_t)S_INACTIVE)
                                ipcSender(sensorInfo[i].self_id, (uint8_t)S_ACTIVE);
                            else if (sensorInfo[i].state == (uint8_t)S_ACTIVE)
                                ipcSender(sensorInfo[i].self_id, (uint8_t)S_INACTIVE);
                            selection = i;
                            swflag = true;
                        }
                    }
                }
            }
        }
        if ((massActivate && massActivateIndex == 0) || (massActivate && massActivateIndex > 0 && !sensorInfoExt[massActivateIndex - 1].ipcResponsePending))
        {
            tft.drawRoundRect(sensorInfoExt[massActivateIndex].touchArea[0] - 1, sensorInfoExt[massActivateIndex].touchArea[1] - 1, sensorInfoExt[massActivateIndex].touchArea[2] + 2, sensorInfoExt[massActivateIndex].touchArea[3] + 2, 5, ILI9341_YELLOW);
            if (selfInfo.state == (uint8_t)S_INACTIVE)
                ipcSender(sensorInfo[massActivateIndex].self_id, (uint8_t)S_ACTIVE);
            else if (selfInfo.state == (uint8_t)S_ACTIVE)
                ipcSender(sensorInfo[massActivateIndex].self_id, (uint8_t)S_INACTIVE);
            selection = massActivateIndex;
            // swflag = true;
            massActivateIndex++;
            if (massActivateIndex == sensorIndex)
            {
                if (selfInfo.state == (uint8_t)S_INACTIVE)
                    selfInfo.state = (uint8_t)S_ACTIVE;
                else if (selfInfo.state == (uint8_t)S_ACTIVE)
                    selfInfo.state = (uint8_t)S_INACTIVE;
                massActivateIndex = 0;
                massActivate = false;
            }
        }

        if (uxQueueMessagesWaiting(manager2GuiDeviceIndexQueue) > 0)
        {
            digitalWrite(STAT_LED, !digitalRead(STAT_LED));
            uint8_t i;
            if (xQueueReceive(manager2GuiDeviceIndexQueue, (void *)&i, 1) == pdPASS)
            {
                uint16_t xsp = ((i % 2) * GUI_PANEL_REL_X_SPACING);
                uint16_t ysp = ((i / 2) * GUI_PANEL_REL_Y_SPACING);
                sensorInfoExt[i].touchArea[0] = GUI_PANEL_START_X_OFFSET + xsp;
                sensorInfoExt[i].touchArea[1] = GUI_PANEL_START_Y_OFFSET + ysp;
                sensorInfoExt[i].touchArea[2] = GUI_PANEL_BOX_WIDTH;
                sensorInfoExt[i].touchArea[3] = GUI_PANEL_BOX_HEIGHT;
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
                default:
                    head = ILI9341_BLUE;
                    bg = ILI9341_DARKGREEN;
                    break;
                }
                tft.setTextColor(head);
                tft.fillRoundRect(GUI_PANEL_START_X_OFFSET + xsp, GUI_PANEL_START_Y_OFFSET + ysp, GUI_PANEL_BOX_WIDTH, GUI_PANEL_BOX_HEIGHT, GUI_PANEL_BOX_CORNER_RADIUS, bg);
                tft.fillRectVGradient(GUI_PANEL_START_X_OFFSET + xsp, GUI_PANEL_START_Y_OFFSET + ysp, GUI_PANEL_BOX_WIDTH, 35, ILI9341_BLACK, bg);
                tft.setFreeFont(&FreeSansBold9pt7b);
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
                tft.setCursor(5 + xsp, 90 + ysp);
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
                    tft.print("INACTIVE");
                    break;
                case S_FAULT_HW:
                    tft.print("HW FAULT");
                    break;
                default:
                    break;
                }

                tft.drawRect(5 + xsp, 110 + ysp, 30, 15, ILI9341_BLACK);
                tft.fillRect(5 + xsp, 110 + ysp, (int16_t)(sensorInfo[i].lqi / 8), 15, ILI9341_BLACK);
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
                        tft.fillRect(40 + xsp, 111 + ysp, 15, 14, ILI9341_BLACK);
                        tft.setCursor(45 + xsp, 115 + ysp);
                        tft.print("!");
                        break;
                    }
                    tft.fillRect(40 + xsp, 123 + ysp - batt * 3, 15, 2, ILI9341_BLACK);
                }
                if (sensorInfoExt[i].alive && (sensorInfo[i].state == S_INACTIVE))
                {
                    tft.setSwapBytes(true);
                    tft.pushImage(60 + xsp, 110 + ysp, 16, 16, loc);
                    tft.setCursor(60 + xsp, 110 + ysp);
                    tft.setTextColor(ILI9341_ORANGE);
                    tft.print("S");
                }
                else if (sensorInfoExt[i].alive && (sensorInfo[i].state == S_ACTIVE))
                {
                    tft.setSwapBytes(true);
                    tft.pushImage(60 + xsp, 110 + ysp, 16, 16, loc2);
                    tft.setCursor(60 + xsp, 110 + ysp);
                    tft.setTextColor(ILI9341_ORANGE);
                    tft.print("S");
                }
                else if (sensorInfoExt[i].alive && (sensorInfo[i].state == S_ALERTING))
                {
                    tft.setSwapBytes(true);
                    tft.pushImage(60 + xsp, 110 + ysp, 16, 16, loc3);
                    tft.setCursor(60 + xsp, 110 + ysp);
                    tft.setTextColor(ILI9341_BLUE);
                    tft.print("S");
                }
            }
        }
        vTaskDelay(1);
    }
}