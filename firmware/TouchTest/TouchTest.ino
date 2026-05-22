#include "ILI9341_t3n.h"
#include <ILI9341_t3n_font_Arial.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define CS_PIN  4
#define TFT_DC  9
#define TFT_CS 10
#define TFT_RST 5
// MOSI=11, MISO=12, SCK=13

#define TIRQ_PIN  3
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);

DMAMEM uint16_t framebuffer[320 * 240];

void setup() {
  Serial.begin(38400);
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setFrameBuffer(framebuffer);
  tft.useFrameBuffer(true);
  
  ts.begin();
  ts.setRotation(1);
  while (!Serial && (millis() <= 1000));
}

boolean wastouched = true;

void loop() {
  boolean istouched = ts.touched();
  if (istouched) {
    TS_Point p = ts.getPoint();
    if (!wastouched) {
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_YELLOW);
      tft.setFont(Arial_60);
      tft.setCursor(60, 80);
      tft.print("Touch");
    }
    tft.fillRect(100, 150, 140, 60, ILI9341_BLACK);
    tft.setTextColor(ILI9341_GREEN);
    tft.setFont(Arial_24);
    tft.setCursor(100, 150);
    tft.print("X = ");
    uint16_t xval = map(p.x, 370, 3850, 0, 320);
    tft.print(xval);
    tft.setCursor(100, 180);
    tft.print("Y = ");
    uint16_t yval = map(p.y, 300, 3800, 0, 240);
    tft.print(yval);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.println(p.y);
  } else {
    if (wastouched) {
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_RED);
      tft.setFont(Arial_48);
      tft.setCursor(120, 50);
      tft.print("No");
      tft.setCursor(80, 120);
      tft.print("Touch");
    }
    Serial.println("no touch");
  }
  wastouched = istouched;
  tft.updateScreen();
  delay(10);
}
