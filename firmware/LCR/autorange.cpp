#include "autorange.h"
#include "board.h"
#include "codec.h"
#include "constants.h"
#include "lcr_func.h"

long unsigned int prev_gain_time = 0;
long unsigned int prev_range_time = 0;


bool gainAutorange(bool force_range) {
  float vpeak = codecReadings.v_peak;
  float ipeak = codecReadings.i_peak;
  bool range_changed = false;

  //If ranging is not forced, check if it has been long enough since last range
  if (!force_range) {
    if (millis() < prev_gain_time + GAIN_AUTORANGE_DELAY) return false;
  }

  if (vpeak > AUTORANGE_LEVEL_HIGH && board.getPGAGainV() != PGA_GAIN_1) {
    board.decreaseVGain();
    range_changed = true;
  } else if (vpeak < AUTORANGE_LEVEL_LOW && board.getPGAGainV() != PGA_GAIN_100) {
    board.increaseVGain();
    range_changed = true;
  }

  if (ipeak > AUTORANGE_LEVEL_HIGH && board.getPGAGainI() != PGA_GAIN_1) {
    if (board.decreaseIGain()) range_changed = true;
  } else if (ipeak < AUTORANGE_LEVEL_LOW && board.getPGAGainI() != PGA_GAIN_100) {
    if (board.increaseIGain()) range_changed = true;
  }

  if (range_changed) {
    prev_gain_time = millis();
    codecResetReadings();
    codecSetReadingsUntilValid(0);
  }
  
  return range_changed;
  
}

bool rangeAutorange(bool force_range) {
  float z_mag = getPhasorMag(calculateZ(), 0);
  bool range_changed = false;

  //If ranging is not forced, check if it has been long enough since last range
  if (!force_range) {
    if (millis() < prev_range_time + RANGE_AUTORANGE_DELAY) return false;
  }
  
  if (z_mag < AUTORANGE_Z_LOW * RANGE_RESISTOR[board.getLCRRange()]) {
    if (board.decreaseLCRRange()) {
      board.increaseIGain();
      board.decreaseVGain();
      board.decreaseVGain();
      range_changed = true;
    }
  } else if (z_mag > AUTORANGE_Z_HIGH * RANGE_RESISTOR[board.getLCRRange()]) {
    if (board.increaseLCRRange()) {
      board.decreaseIGain();
      board.decreaseIGain();
      board.increaseVGain();
      range_changed = true;
    }
  }

  if (range_changed) {
    prev_range_time = millis();
    codecResetReadings();
    codecSetReadingsUntilValid(1);
  }
  
  return range_changed;
}


//Measure repeatedly until system stops auto-ranging
void blockingAutorangeMeasure() {

  uint8_t count = 0;
  
  delay(CODEC_SETTING_CHANGE_DELAY);

  while (count < 3) {
    codecResetReadings();
    
    while (!codecDataAvailable) {
      codecAverageReadings();
    }

    bool range_changed = gainAutorange(true);

    if (range_changed) {
      delay(CODEC_SETTING_CHANGE_DELAY);
    } else {
      count += 1;
    }
    
  }
  
}
