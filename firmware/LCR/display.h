#ifndef _DISPLAY_
#define _DISPLAY_
#include <Arduino.h>

class FloatDisplay {
  public:
    //Set up float to auto-select metric prefix. Hysteresis can be added to prevent rapid switching between prefixes
    void init(uint xpos, uint ypos, bool hysteresis);
    //Set up float with a constant exponent
    void init(uint xpos, uint ypos, int exponent);
    void configSettings(uint digits, int min_exp, float max_value, String unit);
    void updateValue(float value);
              
  private:
    uint xpos;
    uint ypos;
    uint digits;
    float coeff;
    int exponent;
    bool forced_exponent;
    int min_exp;
    float max_value;
    String unit;
    String prefix;
    String text;
    bool hysteresis;
    bool overflow;
            
};

void drawAll(bool force_update = false);

#endif
