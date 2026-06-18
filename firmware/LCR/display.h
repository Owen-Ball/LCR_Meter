#ifndef _DISPLAY_
#define _DISPLAY_
#include <Arduino.h>
#include "lcr_func.h"
#include "ILI9341_t3n.h"
#include <CComplex.h>

class FloatDisplay {
  public:
    //Set up float to auto-select metric prefix. Hysteresis can be added to prevent rapid switching between prefixes
    void init(uint xpos, uint ypos, bool hysteresis);
    //Set up float with a constant exponent
    void init(uint xpos, uint ypos, int exponent);
    void configSettings(uint digits, int min_exp, int min_res, float max_value, const char *unit, const char *label);
    void configSettings(lcr_param_t &params);
    void updateValue(float value);

    //if this is called, every time the digit prints one of the digits will be underlined. Used for selectors
    //Ex: To underline the 5 in 4567.89, this should be 2. For the 8, -1 
    void underlineDigit(int digit);

    void draw(ILI9341_t3n &tft);
              
  private:
    uint xpos;
    uint ypos;
    uint digits;
    float coeff;
    int exponent;
    int prev_leading_digits;
    bool forced_exponent;
    int min_exp;
    int min_resolution;
    float max_value;
    const char *unit;
    const char *label;
    String prefix;
    String text;
    int underlined_digit;
    bool underline;
    String underline_text;
    bool hysteresis;
    bool overflow;
            
};

class LogPrint {
  public:
    void init(ILI9341_t3n &tft, bool serial_logging = false, const GFXfont *font = NULL);
    void setCursor(uint16_t x, uint16_t y, uint16_t line_advance);
    void print(float f);
    void print(Complex c);
    void print(String s);
    void println(float f);
    void println(Complex c);
    void println(String s);
    void println();
    void println_large(String s);

  private:
    uint16_t x;
    uint16_t y;
    uint16_t line_advance;
    ILI9341_t3n *tft;
    const GFXfont *font;
    bool serial_logging;
    
    
};

extern LogPrint logger;

void userPromptText(String s);
void initDraw();
void drawAll(bool force_update = false);

#endif
