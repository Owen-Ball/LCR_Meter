#ifndef _MODESELECTOR_
#define _MODESELECTOR_
#include <Arduino.h>
#include "lcr_func.h"
#include "ILI9341_t3n.h"

class ModeSelector {
  public:
    void init(lcr_param_t **mode_array, uint8_t mode_count);
    void setSelector(uint8_t index);
    void incrementModeUp();
    void incrementModeDown();
    lcr_param_t getPrimary();
    lcr_param_t getSecondary();
    void draw(ILI9341_t3n &tft);
    
    
  private:

    uint8_t current_selector;
    uint8_t primary_index;
    uint8_t secondary_index;
    
    lcr_param_t **mode_array;
    uint8_t mode_count;
};

#endif
