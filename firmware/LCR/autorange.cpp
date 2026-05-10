#include "autorange.h"
#include "board.h"
#include "codec.h"
#include "constants.h"

long prev_gain_time = 0;
long prev_range_time = 0;


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
    board.decreaseIGain();
    range_changed = true;
  } else if (ipeak < AUTORANGE_LEVEL_LOW && board.getPGAGainI() != PGA_GAIN_100) {
    board.increaseIGain();
    range_changed = true;
  }

  if (range_changed) {
    prev_gain_time = millis();
    codecResetReadings();
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
