#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include <Adafruit_ILI9341.h>
#include <XPT2046.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "app_common.h"

/* Objects */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
XPT2046 touch(16, 17);
static uint16_t prev_x = 0xffff, prev_y = 0xffff;
uint8_t selection = 0;

void displayTask(void *pvParameters)
{
    tft.begin();
    touch.begin(240, 320); // Must be done before setting rotation
    tft.fillScreen(ILI9341_BLACK);
    touch.setRotation(touch.ROT180);

    // Replace these for your screen module
    touch.setCalibration(209, 1759, 1775, 273);
    // read diagnostics (optional but can help debug problems)
    tft.setCursor(10, 15);
    tft.setFont(&FreeSansBold12pt7b);

    tft.fillRoundRect(2, 2, 236, 50, 5, ILI9341_ORANGE);
    tft.drawRoundRect(1, 1, 238, 52, 5, ILI9341_BLUE);
    tft.drawRoundRect(0, 0, 240, 54, 5, ILI9341_BLUE);
    tft.drawFastHLine(5, 25, 230, ILI9341_BLACK);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("I2DS Control Panel");
    selfInfoExt.touchArea[0] = 0;
    selfInfoExt.touchArea[1] = 0;
    selfInfoExt.touchArea[2] = 240;
    selfInfoExt.touchArea[3] = 54;
    bool swflag = false;
    while (1)
    {
        if (FLAGipcResponsePending)
        {

            uint16_t color, bg;

            switch (sensorInfo[selection].state)
            {
            case S_INACTIVE:
                color = ILI9341_YELLOW;
                bg = ILI9341_DARKGREY;
                break;
            case S_ALERTING:
                color = ILI9341_YELLOW;
                bg = ILI9341_RED;
                break;
            default:
                color = ILI9341_YELLOW;

                bg = ILI9341_DARKGREEN;
                break;
            }
            Serial.print("BLINK:");
            Serial.println(sensorInfo[selection].self_id);
            tft.drawRoundRect(sensorInfoExt[selection].touchArea[0] - 1, sensorInfoExt[selection].touchArea[1] - 1, sensorInfoExt[selection].touchArea[2] + 2, sensorInfoExt[selection].touchArea[3] + 2, 5, swflag ? color : bg);

            swflag = !swflag;
            vTaskDelay(50);
        }
        if (touch.isTouching() & !FLAGipcResponsePending)
        {
            uint16_t x, y;
            touch.getPosition(x, y);
            for (uint8_t i = 0; i < sensorIndex; i++)
            {
                if ((int16_t)x > sensorInfoExt[i].touchArea[0] && (int16_t)x < (sensorInfoExt[i].touchArea[0] + sensorInfoExt[i].touchArea[2]) &&
                    (int16_t)y > sensorInfoExt[i].touchArea[1] && (int16_t)y < (sensorInfoExt[i].touchArea[1] + sensorInfoExt[i].touchArea[3]))
                {
                    tft.drawRoundRect(sensorInfoExt[i].touchArea[0] - 1, sensorInfoExt[i].touchArea[1] - 1, sensorInfoExt[i].touchArea[2] + 2, sensorInfoExt[i].touchArea[3] + 2, 5, ILI9341_YELLOW);
                    if (sensorInfo[i].state == (uint8_t)S_INACTIVE)
                        ipcSender(sensorInfo[i].self_id, (uint8_t)S_ACTIVE);
                    else
                        ipcSender(sensorInfo[i].self_id, (uint8_t)S_INACTIVE);
                    selection = i;
                }
            }
        }
        if (FLAGguiUpdatePending)
        {
            for (uint8_t i = 0; i < sensorIndex; i++)
            {
                if (sensorInfoExt[i].guiUpdatePending)
                {
                    sensorInfoExt[i].touchArea[0] = 10 + ((i % 2) * 120);
                    sensorInfoExt[i].touchArea[1] = 60 + ((i / 2) * 120);
                    sensorInfoExt[i].touchArea[2] = 100;
                    sensorInfoExt[i].touchArea[3] = 100;
                    Serial.println("DISPLAY");
                    switch (sensorInfo[i].state)
                    {
                    case S_INACTIVE:
                        tft.setTextColor(ILI9341_LIGHTGREY);
                        tft.fillRoundRect(10 + ((i % 2) * 120), 60 + ((i / 2) * 120), 100, 100, 5, ILI9341_DARKGREY);
                        break;
                    case S_ALERTING:
                        tft.setTextColor(ILI9341_BLUE);
                        tft.fillRoundRect(10 + ((i % 2) * 120), 60 + ((i / 2) * 120), 100, 100, 5, ILI9341_RED);
                        break;
                    default:
                        tft.setTextColor(ILI9341_BLUE);
                        tft.fillRoundRect(10 + ((i % 2) * 120), 60 + ((i / 2) * 120), 100, 100, 5, ILI9341_DARKGREEN);
                        break;
                    }
                    tft.setFont(&FreeSansBold9pt7b);
                    tft.setCursor(15 + ((i % 2) * 120), 75 + ((i / 2) * 120));
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
                    tft.setTextColor(ILI9341_WHITE);
                    tft.setCursor(15 + ((i % 2) * 120), 105 + ((i / 2) * 120));
                    tft.print(sensorInfo[i].self_id);
                    tft.setCursor(15 + ((i % 2) * 120), 135 + ((i / 2) * 120));
                    tft.print(sensorInfo[i].state);
                    // tft.setCursor(50, 50 + i * 30);
                    // tft.setTextColor(ILI9341_CYAN);
                    // tft.print("PIRSN");
                    sensorInfoExt[i].guiUpdatePending = false;
                }
            }
            FLAGguiUpdatePending = false;
        }
        delay(50);
    }
}
