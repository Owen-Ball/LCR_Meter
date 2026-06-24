#ifndef _SELECTOR_
#define _SELECTOR_
#include <Arduino.h>
#include "ILI9341_t3n.h"


class Selector {
  public:
    void init(float min_val, float max_val, float init_val);
    void setIncrement(float f);
    float getIncrement();
    void incrementUp();
    void incrementDown();
    float getValue();
    

  private:
    float val;
    float max_val;
    float min_val;
    float increment;
};

#endif
