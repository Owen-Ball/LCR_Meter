#ifndef _CAL_
#define _CAL_

#include <Arduino.h>
#include <CComplex.h>
#include "board.h"
#include <SD.h>


#define CAL_FILE "cal.bin"

//Number of frequencies calibrated at
#define MAX_CAL_POINTS    20

static const int CHIP_SELECT = BUILTIN_SDCARD;

//Magic number to prevent reading an SD card that does not contain calibration data
static const uint32_t CAL_MAGIC = 0xFCA1FCA1;

//Version of the calibration structure. Read in of cal data will abort
//if this does not match
static const uint8_t CAL_VERSION =  1;

static const uint8_t CORR_FREQ_COUNT = 38;

static const float corr_frequencies[CORR_FREQ_COUNT] = {
  10, 20, 30, 40, 50, 60, 80,
  100, 120, 150, 200, 250, 300, 400, 500, 600, 800,
  1000, 1200, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 8000,
  10000, 12000, 15000, 20000, 25000, 30000, 40000, 50000, 60000, 80000, 90000
};

struct CalibrationPoint {
    uint32_t frequency;
    
    Complex v_pga_gain[PGA_GAIN_NUM];
    Complex i_pga_gain[PGA_GAIN_NUM];
    
    Complex tia_gain;
};

extern CalibrationPoint cal_array[MAX_CAL_POINTS];
extern uint8_t cal_points;

bool saveCalibration(const CalibrationPoint* cal_array, uint8_t count);
uint8_t loadCalibration(CalibrationPoint* cal_array);

#endif
