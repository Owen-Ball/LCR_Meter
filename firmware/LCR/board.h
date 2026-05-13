#ifndef _BOARD_
#define _BOARD_

#include <Arduino.h>
#include <MCP9800.h>
#include "constants.h"
#include "buzzer.h"

class Board {
  public:
    void    init();
    void    setLCRRange(uint8_t range);
    void    setPGAGainV(uint8_t gain);
    void    setPGAGainI(uint8_t gain);

    uint8_t getLCRRange() {return board_range; }
    uint8_t getPGAGainV() {return board_gain_v; }
    uint8_t getPGAGainI() {return board_gain_i; }

    bool    increaseVGain();
    bool    decreaseVGain();
    bool    increaseIGain();
    bool    decreaseIGain();
    bool    increaseLCRRange();
    bool    decreaseLCRRange();

    float   getTemperature();

    Buzzer buzzer{BUZZER_PIN};


    
  private:
    uint8_t board_gain_v = PGA_GAIN_1;
    uint8_t board_gain_i = PGA_GAIN_1;
    uint8_t board_range = LCR_RANGE_100;
    MCP9800 temp_sensor;
};



extern Board board;

#endif
