#include "menubar.h"

MenuBar menu;

long prev_t;
bool prev_touch = false;

float freqVal = 0;
float ampVal = 0;

#define CS_PIN  4
#define TFT_DC  9
#define TFT_CS 10
#define TFT_RST 5
#define TIRQ_PIN  3

XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);

DMAMEM uint16_t framebuffer[320 * 240];


void testFunc(){
  Serial.println("executed");
}




void setup() {
  tft.begin(16000000);
  ts.begin();
  ts.setRotation(1);

  
  tft.setRotation(3);
  tft.setFrameBuffer(framebuffer);
  tft.useFrameBuffer(true);
  tft.updateChangedAreasOnly(true);
  
  menu.init(320, 240, 40);

  menu.addCategory("Freq");
  menu.addItem({"100Hz", &freqVal, 100.0f, nullptr, false});
  menu.addItem({"1kHz", &freqVal, 1000.0f, nullptr, false});
  menu.addItem({"10kHz", &freqVal, 10000.0f, nullptr, false});
  
  menu.addCategory("Ampl");
  menu.addItem({"0.1", &ampVal, 0.1f, nullptr, false});
  menu.addItem({"0.5", &ampVal, 0.5f, nullptr, false});
  menu.addItem({"1.0", &ampVal, 1.0f, nullptr, false});
  
  menu.addCategory("Mode");
  menu.addItem({"0.1", &ampVal, 0.1f, nullptr, false});
  menu.addItem({"0.5", &ampVal, 0.5f, nullptr, false});
  menu.addItem({"1.0", &ampVal, 1.0f, nullptr, false});
  
  menu.addCategory("Cal");
  menu.addItem({"0.1", &ampVal, 0.1f, nullptr, false});
  menu.addItem({"0.5", &ampVal, 0.5f, nullptr, false});
  menu.addItem({"1.0", &ampVal, 1.0f, nullptr, false});
  menu.addItem({"2.0", nullptr, 0.0f, &testFunc, true});
  
  menu.toggleCategory(2);

  prev_t = 0;

}


void loop() {
  /*
  // put your main code here, to run repeatedly:
  tft.fillScreen(ILI9341_BLACK);
  menu.toggleCategory(0);
  menu.drawMenu(tft);
  tft.updateScreen();
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  menu.toggleCategory(1);
  menu.drawMenu(tft);
  menu.moveUp();
  tft.updateScreen();
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  menu.toggleCategory(2);
  menu.drawMenu(tft);
  tft.updateScreen();
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  menu.toggleCategory(3);
  menu.drawMenu(tft);
  tft.updateScreen();
  delay(1000);

  for (uint16_t i = 0; i < 1000; i++) {
    Serial.println(millis() - prev_t);
    prev_t = millis();
    
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_RED);   // set color
    tft.setTextSize(2);  
    tft.setCursor(100, 100);
    tft.print(String(.99*sin(millis()/1000.0), 4));
    tft.fillRect(0, 0, 50, 50, 0xfbae);
    tft.setTextColor(ILI9341_WHITE);   // set color
    tft.setTextSize(1);  
    menu.drawMenu(tft);
    tft.updateScreen();
    delay(10);
  }
  */
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_RED);   // set color
    tft.setTextSize(2);  
    tft.setCursor(100, 100);
    tft.print(String(.99*sin(millis()/1000.0), 4));
    tft.setTextColor(ILI9341_WHITE);   // set color
    tft.setTextSize(1);
  boolean istouched = ts.touched();
  if (istouched && !prev_touch) {
    TS_Point p = ts.getPoint();
    uint16_t x = map(p.x, 370, 3850, 0, 320);
    uint16_t y = map(p.y, 300, 3800, 0, 240);
    menu.processTouch(x, y);
  }
  prev_touch = istouched;
  menu.drawMenu(tft);
    tft.updateScreen();
  delay(10);
  Serial.println(freqVal);
}
