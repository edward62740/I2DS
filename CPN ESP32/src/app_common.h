#ifndef APP_CFG_H
#define APP_CFG_H
#include <Arduino.h>
/* Defines */
#define EUSART_IPC_BAUD 115200
#define SPI_DISPLAY_FREQ 48000000






#define MAX_IPC_RESPONSE_TIMEOUT_MS 1000
#define MAX_IPC_REQUEST_RETRY 5
#define MAX_IPC_REQUEST_RESEND 2
#define MAX_PENDING_DEVICEINFO_QUEUE 5
#define XPT_CS 16
#define XPT_IRQ 17
#define XPT_CLK 14
#define XPT_MISO 12
#define XPT_MOSI 13
#define TFT_CS 15
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_RST 4
#define TFT_MISO 35
#define IPC_TX 22
#define IPC_RX 19
#define IPC_HWFC_CTS 21
#define IPC_HWFC_RTS 25
#define LED_1 26
#define LED_2 27
#define PR_PGOOD 33
#define PR_CHG 32


#define GUI_PANEL_BOX_WIDTH 110
#define GUI_PANEL_BOX_HEIGHT 70
#define GUI_PANEL_BOX_CORNER_RADIUS 5
#define GUI_PANEL_START_X_OFFSET 3
#define GUI_PANEL_START_Y_OFFSET 60
#define GUI_PANEL_REL_X_SPACING 120
#define GUI_PANEL_REL_Y_SPACING 80
#endif  // APP_CFG_H
