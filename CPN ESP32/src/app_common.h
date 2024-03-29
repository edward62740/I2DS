#ifndef APP_COMMON_H
#define APP_COMMON_H
#include <Arduino.h>


/* GLOBAL CONFIG */
#define DATABASE_URL "█████████" 
#define API_KEY "█████████"
#define FW_VERSION "v1.2"
#define EUSART_IPC_BAUD 115200
#define SPI_DISPLAY_FREQ 48000000
#define WIFI_SSID "█████████"
#define WIFI_PASSWORD "█████████"
#define DEBUG 1
#ifdef DEBUG
#define DEBUG_LOGS 1
#else
#define DEBUG_LOGS 0
#endif

#define APP_LOG_START()           do { if (DEBUG_LOGS)  Serial.begin(57600); } while (0)
#define APP_LOG_INFO(...) \
            do { if (DEBUG_LOGS)  Serial.println(__VA_ARGS__); } while (0)

/* TIMER DURATIONS */
#define IPC_RESPONSE_TIMEOUT_MS 2000 // Sensor poll interval
#define GUI_HEAD_UPDATE_INTERVAL_MS 2500 // Sensor report interval
#define GUI_SLEEP_IF_NO_TOUCH_MS 600000 // Put display to sleep if no user interaction after this interval
#define GUI_TOUCH_DEBOUNCE_MS 250
#define MANAGER_MAX_DEVICE_NOMSG_MS 15000 // Timeout for device 'alive' status
#define FIREBASE_UPDATE_INTERVAL_MS 5000 // Regular firebase update interval
#define FIREBASE_MAX_COMBINED_REQUEST_DURATION 25000 // (Depreciated)
#define FIREBASE_WIFI_RETRY_INTERVAL_MS 3500 // Wi-Fi reconnection interval 
#define FIREBASE_WIFI_MAX_NO_CONNECTION_MS 30000 // Restarts app if unable to post data to firebase for this duration

/* TASK CONSTS */
#define FIREBASE_DATA_OBJECT_PAYLOAD_SIZE_BYTES 8192
#define MAX_I2DS_DEVICE_COUNT 30
#define MAX_IPC_REQUEST_RETRY 5
#define MAX_IPC_REQUEST_RESEND 2
#define MAX_FIREBASE_SET_RETRY 3
#define MAX_FIREBASE_ERROR_QUEUE 10
#define MAX_PENDING_DEVICEINFO_QUEUE 10
#define MAX_FIREBASE_REQUEST_QUEUE 10

/* PINOUTS */
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
#define TFT_LED 5
#define IPC_TX 22
#define IPC_RX 19
#define IPC_HWFC_CTS 21
#define IPC_HWFC_RTS 25
#define STAT_LED 26
#define ACT_LED 27
#define PR_PGOOD 33
#define PR_CHG 32

/* GUI SPACING CONSTS */
#define GUI_PANEL_BOX_WIDTH 111
#define GUI_PANEL_BOX_HEIGHT 70
#define GUI_PANEL_BOX_CORNER_RADIUS 5
#define GUI_PANEL_START_X_OFFSET 3
#define GUI_PANEL_START_Y_OFFSET 60
#define GUI_PANEL_REL_X_SPACING 120
#define GUI_PANEL_REL_Y_SPACING 75
#define GUI_MAX_ITEMS 6
/*
 <--GUI_PANEL_BOX_WIDTH-->                                 GUI_PANEL_BOX_CORNER_RADIUS
 |↙ OFFSET               |     GUI_PANEL_REL_X_SPACING    ↙ 
╭------------------------╮   ↙ ╭------------------------╮ - - - - ᐱ
|                        |<--->|                        |         |
|                        |     |                        |         |
|                        |     |                        |         |  
|   GUI SPACING GUIDE    |     |                        | GUI_PANEL_BOX_HEIGHT
|                        |     |                        |         |
|                        |     |                        |         |
|                        |     |                        |         |
╰------------------------╯     ╰----------------------ᐱ-╯ - - - - ᐯ
                                                      |   GUI_PANEL_REL_Y_SPACING
╭------------------------╮     ╭----------------------ᐯ-╮
|                        |     |                        |
|                        |     |                        |
|                        |     |                        |
|                        |     |                        |
|                        |     |                        |
|                        |     |                        |
|                        |     |                        |
╰------------------------╯     ╰------------------------╯
*/

/* BITMAPS */
const unsigned short wifilogo[0x294] PROGMEM ={
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0316, 0x0A74, 0x0A74, 0x0A73,   // 0x0010 (16)
0x0A74, 0x0A75, 0x12D6, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0020 (32)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x03EF, 0x1233, 0x1233, 0x1233, 0x1233, 0x1233, 0x1253, 0x1253, 0x1253, 0x1233,   // 0x0030 (48)
0x1233, 0x1233, 0x1233, 0x1233, 0x03EF, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0040 (64)
0xF800, 0xF800, 0x29F2, 0x19F1, 0x19F2, 0x1A12, 0x1A13, 0x1A13, 0x1A12, 0x1A12, 0x1A13, 0x1A13, 0x1A13, 0x1A12, 0x1A12, 0x1A13,   // 0x0050 (80)
0x1A13, 0x1A12, 0x19F1, 0x19F2, 0x21D1, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x21B0,   // 0x0060 (96)
0x21B0, 0x21D2, 0x21D1, 0x21B1, 0x21D1, 0x21B1, 0x19B0, 0x19B0, 0x19B0, 0x21B0, 0x19B0, 0x19B0, 0x19B0, 0x21B1, 0x21D1, 0x21D1,   // 0x0070 (112)
0x21D1, 0x21D2, 0x21D1, 0x21B0, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x29CF, 0x2190, 0x21B1, 0x2190, 0x2190,   // 0x0080 (128)
0x2190, 0x218F, 0x2190, 0x2190, 0x218E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x21AF, 0x2190, 0x2190, 0x2170, 0x2191, 0x2190,   // 0x0090 (144)
0x2190, 0x21B1, 0x2190, 0x218E, 0xF800, 0xF800, 0xF800, 0xF800, 0x214E, 0x214F, 0x2170, 0x214F, 0x2170, 0x216F, 0x216F, 0x38EE,   // 0x00A0 (160)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x38EF, 0x216F, 0x216F, 0x2170, 0x214F,   // 0x00B0 (176)
0x2170, 0x216F, 0x214E, 0xF800, 0xF800, 0x296E, 0x294F, 0x294F, 0x294F, 0x294F, 0x294F, 0x294D, 0xF800, 0xF800, 0xF800, 0x1274,   // 0x00C0 (192)
0x1253, 0x1253, 0x1253, 0x1253, 0x0A53, 0x1253, 0x1254, 0x1253, 0xF800, 0xF800, 0xF800, 0x294D, 0x212E, 0x294F, 0x294F, 0x294F,   // 0x00D0 (208)
0x212F, 0x212E, 0x294E, 0x292E, 0x292F, 0x292F, 0x212E, 0x212E, 0xF800, 0xF800, 0xF800, 0x19F1, 0x1A12, 0x1A12, 0x1A33, 0x1A33,   // 0x00E0 (224)
0x1A33, 0x1A33, 0x1A33, 0x1A33, 0x1A33, 0x1A12, 0x1A12, 0x1A12, 0xF800, 0xF800, 0xF800, 0x294E, 0x292E, 0x292F, 0x292F, 0x292E,   // 0x00F0 (240)
0x290E, 0x2950, 0x292F, 0x292E, 0x292E, 0xF800, 0xF800, 0x21CF, 0x21B0, 0x21D1, 0x21D2, 0x21D1, 0x21D1, 0x21D1, 0x21D1, 0x21B1,   // 0x0100 (256)
0x21D1, 0x21D1, 0x21D1, 0x21D1, 0x21D2, 0x21D1, 0x21B0, 0x21AF, 0xF800, 0xF800, 0x20ED, 0x292E, 0x292F, 0x2950, 0x212D, 0x290E,   // 0x0110 (272)
0x292E, 0x294C, 0xF800, 0xF800, 0x216F, 0x2190, 0x2191, 0x2190, 0x2190, 0x216F, 0x216F, 0x218F, 0x216F, 0x196F, 0x216F, 0x216F,   // 0x0120 (288)
0x216F, 0x216F, 0x2190, 0x2190, 0x2191, 0x2170, 0x216F, 0xF800, 0xF800, 0x294C, 0x292E, 0x212D, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0130 (304)
0xF800, 0x214E, 0x216F, 0x214F, 0x2970, 0x214F, 0x214F, 0x212E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x294F,   // 0x0140 (320)
0x214F, 0x214F, 0x2970, 0x214F, 0x216F, 0x214E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x292E, 0x292F,   // 0x0150 (336)
0x292F, 0x294F, 0x294E, 0x212D, 0xF800, 0xF800, 0xF800, 0xF800, 0x03FA, 0x02B7, 0x03FA, 0xF800, 0xF800, 0xF800, 0xF800, 0x212C,   // 0x0160 (352)
0x292E, 0x294F, 0x292F, 0x292F, 0x292E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x290E, 0x2950, 0x292F, 0x292E,   // 0x0170 (368)
0xF800, 0xF800, 0xF800, 0x1252, 0x1A32, 0x1233, 0x1233, 0x1232, 0x1233, 0x1232, 0x1A13, 0x2231, 0xF800, 0xF800, 0xF800, 0x292E,   // 0x0180 (384)
0x292F, 0x2950, 0x212E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x212D, 0x210E, 0x292E, 0xF800, 0xF800, 0xF800,   // 0x0190 (400)
0x21AF, 0x21D1, 0x21F2, 0x21F2, 0x21D1, 0x21D2, 0x21D2, 0x21F2, 0x21F2, 0x21B1, 0x21B0, 0xF800, 0xF800, 0xF800, 0x292E, 0x292E,   // 0x01A0 (416)
0x18EE, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x216F, 0x2190, 0x2170,   // 0x01B0 (432)
0x2170, 0x2170, 0x214F, 0x214F, 0x214F, 0x2170, 0x2170, 0x2170, 0x2190, 0x216F, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x01C0 (448)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x212D, 0x214F, 0x294F, 0x294F, 0x214E, 0x214E,   // 0x01D0 (464)
0x210C, 0xF800, 0x212D, 0x214E, 0x214E, 0x294F, 0x214F, 0x214F, 0x292E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x01E0 (480)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x290D, 0x290E, 0x292F, 0x292E, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x01F0 (496)
0xF800, 0xF800, 0xF800, 0x292E, 0x292F, 0x210E, 0x212D, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0200 (512)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x292D, 0x292E, 0xF800, 0xF800, 0xF800, 0xF800, 0x1295, 0xF800, 0xF800,   // 0x0210 (528)
0xF800, 0xF800, 0x292E, 0x290D, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0220 (544)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x19F1, 0x1A12, 0x1A34, 0x1A12, 0x19F1, 0xF800, 0xF800,   // 0x0230 (560)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0240 (576)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x294F, 0x198F, 0x21B1, 0x2190, 0x21B1, 0x2190, 0x18EF, 0xF800, 0xF800, 0xF800,   // 0x0250 (592)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0260 (608)
0xF800, 0xF800, 0xF800, 0xF800, 0x18EF, 0x212E, 0x214F, 0x214E, 0x214F, 0x214E, 0x296E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0270 (624)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0280 (640)
0xF800, 0xF800, 0xF800, 0x292E, 0x294F, 0x292F, 0x2950, 0x290E, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0290 (656)
0xF800, 0xF800, 0xF800, 0xF800, };

const unsigned short loc[0x100] PROGMEM ={
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x0010 (16)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x0000, 0x0000, 0x7BEF, 0x7BEF,   // 0x0020 (32)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x0020, 0x7BEF,   // 0x0030 (48)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000,   // 0x0040 (64)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000,   // 0x0050 (80)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000,   // 0x0060 (96)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF,   // 0x0070 (112)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF,   // 0x0080 (128)
0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x0090 (144)
0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x00A0 (160)
0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x00B0 (176)
0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x00C0 (192)
0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x00D0 (208)
0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x00E0 (224)
0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x00F0 (240)
0x7BEF, 0x7BEF, 0x7BEF, 0x0000, 0x0000, 0x0000, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,   // 0x0100 (256)
};

const unsigned short loc2[0x100] PROGMEM ={
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x0010 (16)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x0000, 0x0000, 0x03E0, 0x03E0,   // 0x0020 (32)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x0020, 0x03E0,   // 0x0030 (48)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000,   // 0x0040 (64)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000,   // 0x0050 (80)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000,   // 0x0060 (96)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0,   // 0x0070 (112)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0,   // 0x0080 (128)
0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x0090 (144)
0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x00A0 (160)
0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x00B0 (176)
0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x00C0 (192)
0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x00D0 (208)
0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x00E0 (224)
0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x00F0 (240)
0x03E0, 0x03E0, 0x03E0, 0x0000, 0x0000, 0x0000, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0, 0x03E0,   // 0x0100 (256)
};

const unsigned short loc3[0x100] PROGMEM ={
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0010 (16)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800,   // 0x0020 (32)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0x0020, 0xF800,   // 0x0030 (48)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000,   // 0x0040 (64)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000,   // 0x0050 (80)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000,   // 0x0060 (96)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800,   // 0x0070 (112)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800,   // 0x0080 (128)
0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0x6B4D, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0090 (144)
0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x00A0 (160)
0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x00B0 (176)
0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x00C0 (192)
0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x00D0 (208)
0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x00E0 (224)
0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x00F0 (240)
0xF800, 0xF800, 0xF800, 0x0000, 0x0000, 0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800,   // 0x0100 (256)
};

const unsigned short loc4[0x100] PROGMEM ={
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x0010 (16)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0x0000, 0x0000, 0xF81F, 0xF81F,   // 0x0020 (32)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0x0020, 0xF81F,   // 0x0030 (48)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000,   // 0x0040 (64)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000,   // 0x0050 (80)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000,   // 0x0060 (96)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F,   // 0x0070 (112)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F,   // 0x0080 (128)
0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0x6B4D, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x0090 (144)
0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x00A0 (160)
0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x00B0 (176)
0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x00C0 (192)
0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x00D0 (208)
0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x00E0 (224)
0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x00F0 (240)
0xF81F, 0xF81F, 0xF81F, 0x0000, 0x0000, 0x0000, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F, 0xF81F,   // 0x0100 (256)
};

const unsigned short reload[0x190] PROGMEM ={
0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x2124, 0x2124, 0x2124, 0x18E3, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x0010 (16)
0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x2104, 0x2104, 0x2104, 0x2104, 0x2104, 0x2104, 0x2104,   // 0x0020 (32)
0x2104, 0x2104, 0x18E3, 0x001F, 0x2104, 0x2104, 0x18E3, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x2104, 0x2104, 0x2124, 0x2124,   // 0x0030 (48)
0x2124, 0x2124, 0x2124, 0x2124, 0x2124, 0x2124, 0x2104, 0x2104, 0x2104, 0x2945, 0x2104, 0x001F, 0x001F, 0x001F, 0x001F, 0x2104,   // 0x0040 (64)
0x2124, 0x2124, 0x2124, 0x2124, 0x2104, 0x2104, 0x2104, 0x2124, 0x2124, 0x2124, 0x2124, 0x2124, 0x2104, 0x2124, 0x2104, 0x001F,   // 0x0050 (80)
0x001F, 0x001F, 0x2104, 0x2124, 0x2104, 0x2124, 0x2104, 0x2104, 0x18E3, 0x18E3, 0x18E3, 0x2104, 0x2104, 0x2104, 0x2124, 0x2104,   // 0x0060 (96)
0x2104, 0x2124, 0x2104, 0x001F, 0x001F, 0x2104, 0x2124, 0x2104, 0x2124, 0x2104, 0x2104, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x0070 (112)
0x03E0, 0x2104, 0x2104, 0x2124, 0x2104, 0x2124, 0x2104, 0x001F, 0x10A2, 0x2104, 0x2124, 0x2124, 0x2104, 0x001F, 0x001F, 0x001F,   // 0x0080 (128)
0x001F, 0x001F, 0x001F, 0x001F, 0x2104, 0x2945, 0x2124, 0x2124, 0x2104, 0x2124, 0x2104, 0x10A2, 0x2104, 0x2945, 0x2124, 0x2104,   // 0x0090 (144)
0x2124, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x2104, 0x2124, 0x2104, 0x2104, 0x2104, 0x2124, 0x2104, 0x10A2,   // 0x00A0 (160)
0x18E3, 0x2104, 0x2124, 0x2104, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x18E3, 0x2104,   // 0x00B0 (176)
0x18E3, 0x2124, 0x18E3, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x00C0 (192)
0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x00D0 (208)
0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x00E0 (224)
0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x00F0 (240)
0x001F, 0x18C3, 0x2104, 0x1082, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x0100 (256)
0x2104, 0x2104, 0x2104, 0x2124, 0x18C3, 0x2104, 0x2104, 0x2104, 0x2104, 0x2104, 0x2104, 0x18E3, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x0110 (272)
0x001F, 0x001F, 0x001F, 0x18E3, 0x2124, 0x2124, 0x2124, 0x18E3, 0x18E3, 0x2124, 0x2124, 0x2124, 0x2124, 0x2124, 0x2945, 0x2104,   // 0x0120 (288)
0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x2104, 0x2124, 0x2124, 0x2104, 0x001F, 0x10A2, 0x2104, 0x2124, 0x2104,   // 0x0130 (304)
0x2124, 0x2104, 0x2104, 0x18C3, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x10A2, 0x2104, 0x2124, 0x2104, 0x2124, 0x2104, 0x001F,   // 0x0140 (320)
0x001F, 0x2104, 0x2124, 0x2104, 0x2104, 0x2104, 0x2104, 0x2104, 0x18C3, 0x001F, 0x001F, 0x18E3, 0x2104, 0x2104, 0x2124, 0x2104,   // 0x0150 (336)
0x2124, 0x2104, 0x001F, 0x001F, 0x001F, 0x2104, 0x2124, 0x2104, 0x2124, 0x2104, 0x2124, 0x2124, 0x2104, 0x2104, 0x2104, 0x2104,   // 0x0160 (352)
0x2124, 0x2124, 0x2124, 0x2124, 0x2104, 0x001F, 0x001F, 0x001F, 0x001F, 0x2104, 0x2945, 0x2104, 0x18E3, 0x2124, 0x2124, 0x2124,   // 0x0170 (368)
0x2124, 0x2124, 0x2124, 0x2124, 0x2124, 0x2124, 0x2104, 0x2104, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F, 0x18E3, 0x2124, 0x2104,   // 0x0180 (384)
0x001F, 0x2104, 0x2104, 0x2104, 0x2104, 0x2124, 0x2124, 0x2104, 0x2104, 0x2104, 0x18E3, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,   // 0x0190 (400)
};


#endif  // APP_COMMON_H
