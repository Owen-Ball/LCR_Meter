#ifndef _BOARD_
#define _BOARD_

#include <Arduino.h>
#include <MCP9800.h>
#include "SPI.h"
#include "ILI9341_t3n.h"
#include <XPT2046_Touchscreen.h>
#include "constants.h"
#include "buzzer.h"
#include "Button.h"

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

    //returns true on the loop that the touchscreen was pressed. This should be called at most once per loop
    //Also will set ts_x and ts_y
    bool    tsPressed(long unsigned int debounce_ms = 50);

    void    waitForInput();

    Buzzer buzzer{BUZZER_PIN};
    ILI9341_t3n tft{TFT_CS, TFT_DC, TFT_RST};
    XPT2046_Touchscreen ts{TS_CS, TS_IRQ};
    
    Button enter_button{ENTER_PIN, DEBOUNCE_TIME_MS, BUTTON_AVG_COUNT};
    Button up_button{UP_PIN, DEBOUNCE_TIME_MS, BUTTON_AVG_COUNT};
    Button down_button{DOWN_PIN, DEBOUNCE_TIME_MS, BUTTON_AVG_COUNT};

    Button select_button_0{BUTTON0_PIN, DEBOUNCE_TIME_MS, BUTTON_AVG_COUNT};
    Button select_button_1{BUTTON1_PIN, DEBOUNCE_TIME_MS, BUTTON_AVG_COUNT};
    Button select_button_2{BUTTON2_PIN, DEBOUNCE_TIME_MS, BUTTON_AVG_COUNT};
    Button select_button_3{BUTTON3_PIN, DEBOUNCE_TIME_MS, BUTTON_AVG_COUNT};

    uint16_t ts_x;
    uint16_t ts_y;

    
  private:
  
    uint8_t board_gain_v = PGA_GAIN_1;
    uint8_t board_gain_i = PGA_GAIN_1;
    uint8_t board_range = LCR_RANGE_100;
    MCP9800 temp_sensor;

    bool ts_state;
    long unsigned int ts_ignoreuntil;
};



extern Board board;

#endif
