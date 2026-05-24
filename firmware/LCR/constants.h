#ifndef _CONSTANTS_
#define _CONSTANTS_


//BOARD PINS
#define PGA_V_0_PIN       35
#define PGA_V_1_PIN       36
#define PGA_I_0_PIN       37
#define PGA_I_1_PIN       38
#define RANGE_SEL_0_PIN   34
#define RANGE_SEL_1_PIN   33


#define BUZZER_PIN        14
#define FAN_PIN           15

#define TFT_CS            10
#define TFT_DC            9 
#define TFT_RST           5
#define TS_CS             4
#define TS_IRQ            3

#define ENTER_PIN
#define BACK_PIN
#define UP_PIN
#define DOWN_PIN

#define BUTTON1_PIN
#define BUTTON2_PIN
#define BUTTON3_PIN
#define BUTTON4_PIN


//RANGES
#define LCR_RANGE_100     0
#define LCR_RANGE_1K      1
#define LCR_RANGE_10K     2
#define LCR_RANGE_100K    3
#define LCR_RANGE_NUM     4


//PGA Gains
#define PGA_GAIN_1        0
#define PGA_GAIN_5        1
#define PGA_GAIN_25       2
#define PGA_GAIN_100      3
#define PGA_GAIN_NUM      4



#define LCR_MIN_FREQUENCY   100
#define MAX_DAC_AMPLITUDE   0.95
#define PGA_CAL_AMPLITUDE   0.8
#define DAC_OUTPUT_VPP      4.0


#define CODEC_CLIPPING_LEVEL        0.9
#define CODEC_SETTING_CHANGE_DELAY  500

//Time between autoranges. Removes chance of system not settling at new setting instantly
//and switching to another range
#define GAIN_AUTORANGE_DELAY        150
#define RANGE_AUTORANGE_DELAY       150


//Above this input peak value the board will attempt to drop the PGA gain if possible
#define AUTORANGE_LEVEL_HIGH        0.8
//Below this input peak value the board will attempt to raise the PGA gain. Must be >5x lower than high level
#define AUTORANGE_LEVEL_LOW         0.125

#define AUTORANGE_Z_LOW             0.177   //10^-0.75
#define AUTORANGE_Z_HIGH            5.62    //10^ 0.75

//Unused currently, replaced by more detailed calibration procedure
#define I_PGA_GBWP    40e6
#define V_PGA_GBWP    40e6


//set the resistor used to calibrate each range
//Didn't use 100 and 100k for the lower and upper ranges to avoid impact of probe parasitics
const float RANGE_CAL_RESISTOR[LCR_RANGE_NUM] = {1000, 1000, 10000, 10000};

const float RANGE_RESISTOR[LCR_RANGE_NUM] = {100, 1000, 10000, 100000};


#define V_CAL_RESISTOR    1000.0
#define I_CAL_RESISTOR    10000.0

#define R_OVERFLOW        20e6
#define C_OVERFLOW        20e-3

#define L_OVERFLOW        10







//DISPLAY CONSTANTS

#define DISP_REFRESH_TIME       100

#define SCREEN_WIDTH            320
#define SCREEN_HEIGHT           240

#define DISP_FLOAT_RANGE_UP     1.1
#define DISP_FLOAT_RANGE_DOWN   0.9

#define MENU_CATEGORY_HEIGHT    40
#define MENU_ITEM_HEIGHT        35

//TOUCHSCREEN

#define TS_X_MIN                400
#define TS_X_MAX                3900
#define TS_Y_MIN                250
#define TS_Y_MAX                3750


#endif // _CONSTANTS_
