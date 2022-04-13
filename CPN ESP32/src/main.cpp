#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046.h>

void ipcTask(void *pvParameters);
void firebaseTask(void *pvParameters);

/* Pin Configuration */
#define XPT_CS 16
#define XPT_IRQ 17
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

/* Defines */
#define EUSART_IPC_BAUD 115200
#define SPI_DISPLAY_FREQ 48000000

/* Task Handles */
static TaskHandle_t ipc;
static TaskHandle_t firebase;

/* Objects */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
XPT2046 touch(16, 17);
static uint16_t prev_x = 0xffff, prev_y = 0xffff;

/* IPC Information */

typedef struct
{ /* Sensor info */
  uint8_t hw;
  uint8_t state;
  uint32_t battery_voltage;
  uint8_t node_type;
  uint16_t central_id;
  uint16_t self_id;
  uint8_t endpoint;
  uint8_t trigd;
  uint8_t lqi;
  int8_t rssi;
} DeviceInfo;
DeviceInfo selfInfo, sensorInfo[30];

typedef enum
{                   /* IPC message identification byte */
  IPC_CHANGE,       // (C -> EXT) notify change of information
  IPC_LIST,         // (C -> EXT) send list of connected sensors and associated information
  IPC_REQUEST,      // (C <- EXT) request sensor change state
  IPC_REQUEST_ACK,  // (C -> EXT) ack IPC_REQUEST
  IPC_REQUEST_DONE, // (C -> EXT) finished IPC_REQUEST
  IPC_LIST_CTS,     // (C <- EXT) ack IPC_CHANGE and request IPC_LIST
  IPC_ERR,
} ipc_message_pid_t;

void setup()
{ /*
   Serial.begin(9600);
   Serial.println("ILI9341 Test!");

   tft.begin();
 touch.begin(240, 320);  // Must be done before setting rotation
 tft.fillScreen(ILI9341_BLACK);
   touch.setRotation(touch.ROT180);


   // Replace these for your screen module
   touch.setCalibration(209, 1759, 1775, 273);
   // read diagnostics (optional but can help debug problems)
   uint8_t x = tft.readcommand8(ILI9341_RDMODE);
   Serial.print("Display Power Mode: 0x");
   Serial.println(x, HEX);
   x = tft.readcommand8(ILI9341_RDMADCTL);
   Serial.print("MADCTL Mode: 0x");
   Serial.println(x, HEX);
   x = tft.readcommand8(ILI9341_RDPIXFMT);
   Serial.print("Pixel Format: 0x");
   Serial.println(x, HEX);
   x = tft.readcommand8(ILI9341_RDIMGFMT);
   Serial.print("Image Format: 0x");
   Serial.println(x, HEX);
   x = tft.readcommand8(ILI9341_RDSELFDIAG);
   Serial.print("Self Diagnostic: 0x");
   Serial.println(x, HEX);*/
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, IPC_RX, IPC_TX);

  xTaskCreatePinnedToCore( // Use xTaskCreate()
      ipcTask,             // Function to be called
      "IPC Task",          // Name of task
      32768,               // Stack size
      NULL,                // Parameter to pass
      1,                   // Task priority
      &ipc,                // Task handle
      1);                  // CPU Core

  xTaskCreatePinnedToCore( // Use xTaskCreate()
      firebaseTask,        // Function to be called
      "Firebase Task",     // Name of task
      32768,               // Stack size
      NULL,                // Parameter to pass
      1,                   // Task priority
      &firebase,           // Task handle
      0);                  // CPU Core
  vTaskDelete(NULL);
}

void ipcTask(void *pvParameters)
{
  while (1)
  {
    if (Serial.available())
    { // If anything comes in Serial (USB),
      digitalWrite(LED_1, !digitalRead(LED_1));

      Serial1.write(Serial.read()); // read it and send it out Serial1 (pins 0 & 1)
    }

    if (Serial1.available())
    { // If anything comes in Serial1 (pins 0 & 1)
      digitalWrite(LED_2, !digitalRead(LED_2));

      Serial.write(Serial1.read()); // read it and send it out Serial (USB)
    }
  }
}

void firebaseTask(void *pvParameters)
{
  while (1)
  {
    delay(1);
  }
}

void loop()
{
}
/*
  if (touch.isTouching()) {

    uint16_t x, y;
    touch.getPosition(x, y);
    Serial.println(x);
    if (prev_x == 0xffff) {
      tft.drawPixel(x, y, 0x001f);
    } else {
      tft.drawLine(prev_x, prev_y, x, y, 0x001f);
    }
    prev_x = x;
    prev_y = y;
  } else {
    prev_x = prev_y = 0xffff;
  }
  delay(20);
}*/
