#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046.h>
#define TFT_CS 15
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_RST 4
#define TFT_MISO 35
// ILI9341 with 8-bit parallel interface:
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
XPT2046 touch(16, 17);
static uint16_t prev_x = 0xffff, prev_y = 0xffff;
void setup()
{/*
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
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 19, 22);

}




void loop() {

  if (Serial.available()) {      // If anything comes in Serial (USB),

    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)

  }

  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)

    Serial.write(Serial1.read());   // read it and send it out Serial (USB)

  }
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
