#ifndef _BOARD_
#define _BOARD_

#include <Arduino.h>
#include <MCP9800.h>
#include "buzzer.h"

#define PGA_V_0_PIN       35
#define PGA_V_1_PIN       36
#define PGA_I_0_PIN       37
#define PGA_I_1_PIN       38
#define RANGE_SEL_0_PIN   34
#define RANGE_SEL_1_PIN   33

#define BUZZER_PIN        14
#define FAN_PIN           15

#define ENTER_PIN
#define BACK_PIN
#define UP_PIN
#define DOWN_PIN

#define BUTTON1_PIN
#define BUTTON2_PIN
#define BUTTON3_PIN
#define BUTTON4_PIN

#define LCR_RANGE_100     0
#define LCR_RANGE_1K      1
#define LCR_RANGE_10K     2
#define LCR_RANGE_100K    3
#define LCR_RANGE_NUM     4

#define PGA_GAIN_1        0
#define PGA_GAIN_5        1
#define PGA_GAIN_25       2
#define PGA_GAIN_100      3
#define PGA_GAIN_NUM      4



class Board {
  public:
    void    init();
    void    setLCRRange(uint8_t range);
    void    setPGAGainV(uint8_t gain);
    void    setPGAGainI(uint8_t gain);

    uint8_t getLCRRange() {return board_range; }
    uint8_t getPGAGainV() {return board_gain_v; }
    uint8_t getPGAGainI() {return board_gain_i; }

    void    increaseVGain();
    void    decreaseVGain();
    void    increaseIGain();
    void    decreaseIGain();

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
