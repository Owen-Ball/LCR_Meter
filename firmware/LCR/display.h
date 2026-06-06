#ifndef _DISPLAY_
#define _DISPLAY_
#include <Arduino.h>
#include "lcr_func.h"
#include "ILI9341_t3n.h"

class FloatDisplay {
  public:
    //Set up float to auto-select metric prefix. Hysteresis can be added to prevent rapid switching between prefixes
    void init(uint xpos, uint ypos, bool hysteresis);
    //Set up float with a constant exponent
    void init(uint xpos, uint ypos, int exponent);
    void configSettings(uint digits, int min_exp, float max_value, const char *unit, const char *label);
    void configSettings(lcr_param_t &params);
    void updateValue(float value);

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
    float max_value;
    const char *unit;
    const char *label;
    String prefix;
    String text;
    bool hysteresis;
    bool overflow;
            
};

void initDraw();
void drawCurrentRanges();
void drawLCRReadings();
void drawAll(bool force_update = false);

#endif
