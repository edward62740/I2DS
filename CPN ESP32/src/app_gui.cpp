#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include <XPT2046.h>
#include "app_common.h"
#include "TFT_eSPI.h"

extern "C"
{
    uint8_t temprature_sens_read();
}

/* Objects */
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
XPT2046 touch(16, 17);
static uint16_t prev_x = 0xffff, prev_y = 0xffff;
uint8_t selection = 0;
QueueHandle_t manager2GuiDeviceIndexQueue;
TFT_eSPI tft = TFT_eSPI();
void displayTask(void *pvParameters)
{
    touch.begin(240, 320); // Must be done before setting rotation
    tft.begin();

    tft.fillScreen(ILI9341_BLACK);
    touch.setRotation(touch.ROT180);
    // Replace these for your screen module
    touch.setCalibration(209, 1759, 1775, 273);

    // read diagnostics (optional but can help debug problems)
    tft.setCursor(10, 22);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold12pt7b);
    tft.fillRectHGradient(2, 2, 236, 50, ILI9341_ORANGE, ILI9341_RED);
    // tft.fillSmoothRoundRect(2, 2, 236, 50, 5, ILI9341_ORANGE);

    tft.drawRoundRect(1, 1, 238, 52, 3, ILI9341_BLUE);
    tft.drawFastHLine(5, 25, 230, ILI9341_BLACK);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("I2DS Control Panel");
    tft.setTextFont(1);
    tft.setCursor(10, 30);
    tft.print("Temp: ");
    tft.setCursor(35, 30);
    tft.fillRoundRect(200, 290, 40, 30, 3, ILI9341_BLUE);
    selfInfoExt.touchArea[0] = 0;
    selfInfoExt.touchArea[1] = 0;
    selfInfoExt.touchArea[2] = 240;
    selfInfoExt.touchArea[3] = 54;
    bool swflag = false;
    while (1)
    {
        if (FLAGipcResponsePending || swflag)
        {
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
            Serial.print("BLINK:");
            Serial.println(sensorInfo[selection].self_id);
            tft.drawRoundRect(sensorInfoExt[selection].touchArea[0] - 1, sensorInfoExt[selection].touchArea[1] - 1, sensorInfoExt[selection].touchArea[2] + 2, sensorInfoExt[selection].touchArea[3] + 2, 5, swflag ? ILI9341_BLACK : color);
            swflag = !swflag;
            vTaskDelay(50);
        }
        if (touch.isTouching() & !FLAGipcResponsePending)
        {
            Serial.println("TOUCHING");

            uint16_t x, y;
            touch.getPosition(x, y);
            Serial.println(x);
            Serial.print(y);
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
                    }
                }
            }
        }

        if (uxQueueMessagesWaiting(manager2GuiDeviceIndexQueue) > 0)
        {
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
                case S_INACTIVE:
                    tft.print("INACTIVE");
                    break;
                default:
                    break;
                }
                // tft.setCursor(5 + xsp, 105 + ysp);
                // String battery = "Battery: " + (String)sensorInfo[i].battery_voltage + "%";
                // tft.print(battery);
                // Serial.println(sensorInfo[i].lqi);
                tft.drawRect(5 + xsp, 110 + ysp, 30, 15, ILI9341_BLACK);
                tft.fillRect(5 + xsp, 110 + ysp, (int16_t)(sensorInfo[i].lqi / 8), 15, ILI9341_BLACK);
                tft.fillTriangle(5 + xsp, 110 + ysp, 5 + xsp, 110 + ysp + 15, 5 + xsp + 30, 110 + ysp, bg);
                tft.setTextFont(1);
                tft.setCursor(5 + xsp, 110 + ysp);
                tft.print("LQI");
                tft.fillRect(60 + xsp, 109 + ysp, 5, 2, ILI9341_BLACK);
                tft.drawRect(55 + xsp, 111 + ysp, 15, 14, ILI9341_BLACK);
                for (uint32_t batt = 0; 2000 + (batt * 250) < sensorInfo[i].battery_voltage; batt++)
                {
                    if (batt > 4)
                    {
                        tft.fillRect(55 + xsp, 111 + ysp, 15, 14, ILI9341_BLACK);
                        tft.setCursor(60 + xsp, 116 + ysp);
                        tft.print("!");
                        break;
                    }
                    tft.fillRect(55 + xsp, 123 + ysp - batt * 3, 15, 2, ILI9341_BLACK);
                }

                // tft.fillRect(40 + xsp, 137 + ysp, 15, 3, ILI9341_BLACK);
                // tft.fillRectHGradient(5 +xsp, 135 + ysp, );
                //  tft.setCursor(50, 50 + i * 30);
                //  tft.setTextColor(ILI9341_CYAN);
                //  tft.print("PIRSN");
            }
        }
        vTaskDelay(5);
    }
}
