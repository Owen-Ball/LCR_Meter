#ifndef _CAL_
#define _CAL_

#include <Arduino.h>
#include <CComplex.h>
#include "board.h"
#include <SD.h>


#define CAL_FILE "cal.bin"

//Max number of calibration points that can be read in from a file
#define MAX_CAL_POINTS    20

static const int CHIP_SELECT = BUILTIN_SDCARD;

//Magic number to prevent reading an SD card that does not contain calibration data
static const uint32_t CAL_MAGIC = 0xFCA1FCA1;

//Version of the calibration structure. Read in of cal data will abort
//if this does not match
static const uint8_t CAL_VERSION =  1;

static const uint8_t CAL_FREQ_COUNT = 4;

static const float cal_frequencies[CAL_FREQ_COUNT] = {
  100, 1000, 10000, 75000
};

struct CalibrationPoint {
    uint32_t frequency;
    
    Complex v_pga_gain[PGA_GAIN_NUM];
    Complex i_pga_gain[PGA_GAIN_NUM];
    Complex tia_gain[LCR_RANGE_NUM];
    Complex probe_Zs;
    Complex probe_Zp;
};

//Upon setting a frequency, this will be loaded with the calibration data corresponding to that frequency
extern CalibrationPoint calibration_data;

void printCalibrationPoint(CalibrationPoint& cal_data);
void loadCalibrationPoint(float freq);
bool saveCalibration();
uint8_t loadCalibration();


void calibrateIPGA();
void calibrateVPGA();

void calibrateAll();
void calibrateProbeQuick();

#endif
