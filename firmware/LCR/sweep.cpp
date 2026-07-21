#include "sweep.h"

#include "lcr_func.h"
#include "autorange.h"
#include "constants.h"
#include "LCR_Fonts/Font5x7FixedMono.h"
#include "LCR_Fonts/FreeMono9pt7b.h"
#include "calibration.h"
#include "display.h"



enum display_mode_t {ZMAG_ZPHASE, R_X};

uint16_t num_points = 0;
uint16_t current_points = 0;
uint16_t num_valid_points = 0;
display_mode_t display_mode;


lcr_param_t left_param;
float left_min;
float left_max;
bool left_log_scale;
bool left_neg;
float left_increment;
bool left_discard_zero;

lcr_param_t right_param;
float right_min;
float right_max;
bool right_log_scale;
bool right_neg;
float right_increment;
bool right_phase;
bool right_discard_zero;

uint8_t vert_divs;

float freq_array[FREQSWEEP_MAX_POINTS];
Complex value_array[FREQSWEEP_MAX_POINTS];

//should span 1 to 10
float linear_step_options[6] = {1.0, 2.0, 2.5, 4.0, 5.0, 10.0};
uint8_t linear_step_option_count = 6;

float phase_step_options[10] = {1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 45.0, 60.0, 90.0, 180.0};
uint8_t phase_step_option_count = 10;

void setFreqSweepPoints(float points) {
  num_points = round(points);
}


void setFreqSweepDisplayMode(float mode) {
  
  switch(int(round(mode))) {
    case 0:
      display_mode = ZMAG_ZPHASE;
      left_param = lcrParamZMag;
      right_param = lcrParamZPhase;
      left_discard_zero = false;
      right_discard_zero = false;
      break;
    case 1:
      display_mode = R_X;
      left_param = lcrParamRs;
      right_param = lcrParamXs;
      left_discard_zero = true;
      right_discard_zero = false;
      break;
    default:
      display_mode = ZMAG_ZPHASE;
      left_param = lcrParamZMag;
      right_param = lcrParamZPhase;
      break;   
  }
  
}

float getArrayMin(float (*func)(Complex z, float f), bool discard_zero = false) {
  float min_val = INFINITY;
  for (uint16_t i=0; i<current_points; i++) {
    float val = func(value_array[i], freq_array[i]);
    if (!discard_zero || val != 0.0) {
      if (val < min_val && value_array[i].modulus() < Z_OVERFLOW) min_val = val;
    }
    
  }
  
  return min_val;
}

float getArrayMax(float (*func)(Complex z, float f)) {
  float max_val = -1.0*INFINITY;
  for (uint16_t i=0; i<current_points; i++) {
    float val = func(value_array[i], freq_array[i]);
    //Serial.println(value_array[i].modulus());
    if (val > max_val && value_array[i].modulus() < Z_OVERFLOW) max_val = val;
  }
  return max_val;
}


Complex measurePoint(float freq) {
  setLCRFrequency(freq);
  blockingAutorangeMeasureFast();
  return calculateZ();  
}


int16_t mapFreqToX(float freq, float freq_start, float freq_end, int16_t x0, int16_t width) {
  float freq_span = log10(freq_end / freq_start);
  float x_pos = float(x0) + log10(freq / freq_start) / freq_span * float(width);
  return int16_t(x_pos);
}

int16_t mapValToY(float val, float val_min, float val_max, int16_t y0, int16_t height) {
  if (val < val_min || val > val_max) return INT16_MAX;
  return int16_t(-1*float(height)*(val - val_min)/(val_max - val_min)) + y0;
}

void printPowerOfTen(ILI9341_t3n &tft, int16_t x, int16_t y, int8_t exponent, bool neg = false) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&Font5x7FixedMono);

  tft.setCursor(x, y);
  if (neg) tft.print(-10);
  else tft.print(10);
  tft.setCursor(x + 13 + neg*6, max(0, y - 5));
  tft.print(exponent);
  
}

void printSciNotation(ILI9341_t3n &tft, int16_t x, int16_t y, float val) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&Font5x7FixedMono);

  bool neg = val < 0.0;
  val = abs(val);

  int exponent;
  float coeff;

  if (val == 0) {
    exponent = 0;
    coeff = 0.0;
    
  } else {
    exponent = floor(log10(val));
    coeff = val / pow(10.0, exponent);
    if (neg) coeff *= -1.0;
  }

  tft.setCursor(x, y);
  if (coeff > 9.994) {
    coeff = 1.0;
    exponent += 1;
  }
  
  tft.print(String(coeff, 2));

  if (neg) {
    tft.fillCircle(x + 35, y - 1, 1, ILI9341_WHITE);
    tft.setCursor(x + 38, y);
    tft.print(10);
    tft.setCursor(x + 51, max(0, y - 5));
    tft.print(exponent);
  } else {
    tft.fillCircle(x + 27, y - 1, 1, ILI9341_WHITE);
    tft.setCursor(x + 30, y);
    tft.print(10);
    tft.setCursor(x + 43, max(0, y - 5));
    tft.print(exponent);
  }
}

void printPhase(ILI9341_t3n &tft, int16_t x, int16_t y, float phase) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&Font5x7FixedMono);

  tft.setCursor(x, y);
  tft.print(String(phase, 1));
  
}

float calculateIncrement(float target_increment) {
  int exponent = floor(log10(target_increment));
  float coeff = target_increment / pow(10.0, exponent);

  for (uint8_t i = 0; i < linear_step_option_count; i++) {
    if (coeff < linear_step_options[i]) return linear_step_options[i] * pow(10.0, exponent);
  }
 
  return 10.0 * pow(10.0, exponent);
}


float approximatelyEqual(float f1, float f2) {
  return abs(f1 - f2) < 0.0001;
}


float increaseIncrement(float increment) {
  int exponent = floor(log10(increment));
  float coeff = increment / pow(10.0, exponent);


  for (uint8_t i=0; i < linear_step_option_count; i++ ){
    if (approximatelyEqual(coeff, linear_step_options[i])) {
      if (i == linear_step_option_count - 1) coeff = 10.0 * linear_step_options[1];
      else coeff = linear_step_options[i+1];

      return coeff * pow(10.0, exponent);
    }
  }
}


float getLinearIncrement(float min_val, float max_val, uint8_t divs) {

  if (approximatelyEqual(min_val, max_val)) return 0.001;

  float increment = calculateIncrement((max_val - min_val) / divs);

  bool complete = false;

  //Does this need to be a loop? No. Does it work? Yes
  //Loop while the selected increment is not large enough to span the data range
  while (true) {
    float div_min = floor(min_val / increment) * increment;
    float div_max = increment*divs + div_min;

    if (div_max > max_val) return increment;
    else increment = increaseIncrement(increment);
  }

  return increment;
  
}

float getPhaseIncrement(float min_val, float max_val, uint8_t divs) {
  for (uint8_t i = 0; i < phase_step_option_count; i++ ) {

    float increment = phase_step_options[i];
    float div_min = floor(min_val / increment) * increment;
    float div_max = increment*divs + div_min;

    if (div_max > max_val) return increment;
  }

  return phase_step_options[phase_step_option_count - 1];
}

void scaleVerticalAxes() {
  float left_val_min = getArrayMin(left_param.value, left_discard_zero);
  float left_val_max = getArrayMax(left_param.value);

  float right_val_min = getArrayMin(right_param.value, right_discard_zero);
  float right_val_max = getArrayMax(right_param.value);

  if (right_param.label == lcrParamZPhase.label) {
    right_log_scale = false;
    right_phase = true;
  } else {
    right_log_scale = true;
    right_phase = false;
  }

  left_log_scale = true;

  if (left_val_max <= 0) {
    left_neg = true;
    std::swap(left_val_min, left_val_max);
    left_val_min *= -1.0;
    left_val_max *= -1.0;
  }

  if (right_val_max <= 0) {
    right_neg = true;
    std::swap(right_val_min, right_val_max);
    right_val_min *= -1.0;
    right_val_max *= -1.0;
  }

  if (left_log_scale && left_val_min < 0) left_log_scale = false;
  else if (left_log_scale && left_val_min < 1e-3) left_val_min = 1e-3;

  if (right_log_scale && right_val_min < 0) right_log_scale = false;
  else if (right_log_scale && right_val_min < 1e-3) right_val_min = 1e-3;

  uint8_t left_divs = 0;
  uint8_t right_divs = 0;
  uint8_t divs = 0;

  if (left_log_scale) {
    left_divs = ceil(log10(left_val_max)) - floor(log10(left_val_min));
    if (left_divs <= 1) left_log_scale = false;
    left_min = floor(log10(left_val_min));
  }

  if (right_log_scale) {
    right_divs = ceil(log10(right_val_max)) - floor(log10(right_val_min));
    if (right_divs <= 1) right_log_scale = false;
    right_min = floor(log10(right_val_min));
  }


  if (left_neg && !left_log_scale) {
    left_neg = false;
    std::swap(left_val_min, left_val_max);
    left_val_min *= -1.0;
    left_val_max *= -1.0;
  }

  
  if (right_neg && !right_log_scale) {
    right_neg = false;
    std::swap(right_val_min, right_val_max);
    right_val_min *= -1.0;
    right_val_max *= -1.0;
  }
  

  if (left_log_scale && right_log_scale) {
    divs = max(left_divs, right_divs);
    left_max = left_min + divs;
    right_max = right_min + divs;
    
  } else if (left_log_scale && !right_log_scale) {
    divs = left_divs;
    left_max = left_min + divs;
    if (right_phase) right_increment = getPhaseIncrement(right_val_min, right_val_max, divs);
    else right_increment = getLinearIncrement(right_val_min, right_val_max, divs);
    right_min = floor(right_val_min / right_increment) * right_increment;
    right_max = right_min + right_increment*divs;

  } else if (!left_log_scale && right_log_scale) {
    divs = right_divs;
    right_max = right_min + divs;
    left_increment = getLinearIncrement(left_val_min, left_val_max, divs);
    left_min = floor(left_val_min / left_increment) * left_increment;
    left_max = left_min + left_increment*divs;
    
  } else {
    
    divs = 4;

    left_increment = getLinearIncrement(left_val_min, left_val_max, divs);
    left_min = floor(left_val_min / left_increment) * left_increment;
    left_max = left_min + left_increment*divs;

    if (right_phase) right_increment = getPhaseIncrement(right_val_min, right_val_max, divs);
    else right_increment = getLinearIncrement(right_val_min, right_val_max, divs);
    right_min = floor(right_val_min / right_increment) * right_increment;
    right_max = right_min + right_increment*divs;
    
  }

  
  if (left_val_min == INFINITY || left_val_min == INFINITY) {
    left_min = 0.0;
    left_max = 0.0;
    left_increment = 0.0;
  }

  if (right_val_min == INFINITY || right_val_min == INFINITY) {
    right_min = 0.0;
    right_max = 0.0;
    right_increment = 0.0;
  }
  

  Serial.print("left min: ");
  Serial.print(left_val_min);
  Serial.print(", ");
  Serial.println(floor(log10(left_val_min)));
  Serial.print("left max:");
  Serial.print(left_val_max);
  Serial.print(", ");
  Serial.println(ceil(log10(left_val_max)));
  
  Serial.print("right min: ");
  Serial.print(right_val_min);
  Serial.print(", ");
  Serial.println(floor(log10(right_val_min)));
  Serial.print("right max:");
  Serial.print(right_val_max);
  Serial.print(", ");
  Serial.println(ceil(log10(right_val_max)));

  
  Serial.println(divs);

  vert_divs = divs;
  
}


void drawThickLine(ILI9341_t3n &tft, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  tft.drawLine(x0, y0+2, x1, y1+2, color);
  tft.drawLine(x0, y0+1, x1, y1+1, color);
  tft.drawLine(x0, y0, x1, y1, color);
  tft.drawLine(x0, y0-1, x1, y1-1, color);
  tft.drawLine(x0, y0-2, x1, y1-2, color);
}

void drawPlot(ILI9341_t3n &tft, float sweep_start, float sweep_end) {

  int16_t x;
  int16_t y;

  int8_t exp_start = floor(log10(round(sweep_start)));
  int8_t exp_end = ceil(log10(round(sweep_end)));
  float freq_start = pow(10, exp_start);
  float freq_end = pow(10, exp_end);
  int16_t x0 = SCREEN_WIDTH/2 - FREQSWEEP_WIDTH/2;
  int16_t y0 = SCREEN_HEIGHT/2 - FREQSWEEP_HEIGHT/2 + FREQSWEEP_Y_OFFSET;
  
  int16_t x1 = x0 + FREQSWEEP_WIDTH;
  int16_t y1 = y0 + FREQSWEEP_HEIGHT;
  

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  board.tft.setFont(&FreeMono9pt7b);
  
  x = getCenteredXPos("Frequency (Hz)");
  tft.setCursor(x, y1 + 20);
  tft.print("Frequency (Hz)");

  tft.setTextColor(ILI9341_RED);
  String left_text = String(left_param.label) + "(" + left_param.unit + ")";
  x = getCenteredXPos(left_text, SCREEN_WIDTH / 3 - 5);
  tft.setCursor(x, y0 - 20);
  tft.print(left_text);

  tft.setTextColor(ILI9341_BLUE);
  String right_text = String(right_param.label) + "(" + right_param.unit + ")";
  x = getCenteredXPos(right_text, 2 * SCREEN_WIDTH / 3 - 5);
  tft.setCursor(x, y0 - 20);
  tft.print(right_text);
  

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&Font5x7FixedMono);


  tft.drawLine(x0, y0, x0, y1, ILI9341_WHITE);
  tft.drawLine(x0, y1, x1, y1, ILI9341_WHITE);


  
  for (int8_t exp = exp_start; float(exp) <= log10(freq_end + 1.0); exp += 1) {

    for (uint8_t i=1; i<FREQSWEEP_DIVS_PER_DECADE; i++) {
      float freq = pow(10, exp)*(10.0/FREQSWEEP_DIVS_PER_DECADE)*float(i);
      if (freq > freq_end) break;
      x = mapFreqToX(freq, freq_start, freq_end, x0, FREQSWEEP_WIDTH - FREQSWEEP_PADDING);
      tft.drawLine(x, y0, x, y1-1, 0x5acb);
    }

    x = mapFreqToX(pow(10, exp), freq_start, freq_end, x0, FREQSWEEP_WIDTH - FREQSWEEP_PADDING);
    tft.drawLine(x, y0, x, y1, ILI9341_WHITE);
    printPowerOfTen(tft, x-7, y1+12, exp);

    
  }


  if (num_valid_points <= 1) {
    return;
  }
  
  scaleVerticalAxes();
  
  for (int8_t div = 0; div <= vert_divs; div++) {
    y = y1 - (div * (FREQSWEEP_HEIGHT - FREQSWEEP_PADDING)) / vert_divs;
    tft.drawLine(x0, y, x1, y, ILI9341_WHITE);

    if (left_log_scale) {
      if (left_neg) printPowerOfTen(tft, x0 - 36, y, left_min + div, true);
      else printPowerOfTen(tft, x0 - 30, y, left_min + div);
    } else {
      printSciNotation(tft, x0 - 58, y, left_min + left_increment*div);
    }

    if (right_log_scale) {
      if (right_neg) printPowerOfTen(tft, x1 + 5, y, right_min + div, true);
      else printPowerOfTen(tft, x1 + 5, y, right_min + div);
    } else if (!right_phase){
      printSciNotation(tft, x1 + 5, y, right_min + right_increment*div);
    } else {
      printPhase(tft, x1 + 5, y, right_min + right_increment*div);
    }
  }


  int16_t prev_x_left = INT16_MAX;
  int16_t prev_y_left = INT16_MAX;
  int16_t prev_x_right = INT16_MAX;
  int16_t prev_y_right = INT16_MAX;
  
  for (uint16_t i=0; i<current_points; i++) {
    float right_val = right_param.value(value_array[i], freq_array[i]);
    x = mapFreqToX(freq_array[i], freq_start, freq_end, x0, FREQSWEEP_WIDTH - FREQSWEEP_PADDING);
    if (right_log_scale) {
      y = mapValToY(log10(max(abs(right_val), 1e-3)), right_min, right_max, y1, FREQSWEEP_HEIGHT - FREQSWEEP_PADDING);
    } else {
      y = mapValToY(right_val, right_min, right_max, y1, FREQSWEEP_HEIGHT - FREQSWEEP_PADDING);
    }


    if (y != INT16_MAX) {
      tft.fillCircle(x, y, 2, ILI9341_BLUE);
      if (prev_y_right != INT16_MAX) {
        //drawThickLine(tft, x, y, prev_x_right, prev_y_right, ILI9341_BLUE);
      }
      prev_x_right = x;
      prev_y_right = y; 
      
    } else {
      prev_x_right = INT16_MAX;
      prev_y_right = INT16_MAX;
    }
    





    float left_val = left_param.value(value_array[i], freq_array[i]);
    if (left_log_scale) {
      y = mapValToY(log10(max(abs(left_val), 1e-3)), left_min, left_max, y1, FREQSWEEP_HEIGHT - FREQSWEEP_PADDING);
    } else {
      y = mapValToY(left_val, left_min, left_max, y1, FREQSWEEP_HEIGHT - FREQSWEEP_PADDING);
    }

    
    if (y != INT16_MAX) {
      tft.fillCircle(x, y, 2, ILI9341_RED);
      if (prev_y_left != INT16_MAX) {
        //drawThickLine(tft, x, y, prev_x_left, prev_y_left, ILI9341_RED);
      }
      prev_x_left = x;
      prev_y_left = y; 
      
    } else {
      prev_x_left = INT16_MAX;
      prev_y_left = INT16_MAX;
    }
    
  }

  
  
}

void drawSweepWrapper(ILI9341_t3n &tft) {
  drawPlot(tft, FREQSWEEP_START, FREQSWEEP_END);
}


void runSweep(float freq_start, float freq_end, uint16_t points) {
  
  float freq_step = pow(freq_end / freq_start, 1.0 / (float(points) - 1.0));
  current_points = 0;
  num_valid_points = 0;

  float freq = freq_start;
  for (uint16_t i=0; i<points; i++) {
    Complex val = measurePoint(freq);

    freq_array[i] = freq;
    value_array[i] = val;

    current_points = i + 1;

    if (val.modulus() < Z_OVERFLOW) num_valid_points += 1;

    Serial.print(freq);
    Serial.print(" ");
    Serial.println(val.real());
    
    freq *= freq_step;

    if (i > 0 && i % 5 == 0) {
      board.tft.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - MENU_CATEGORY_HEIGHT, ILI9341_BLACK);
      drawPlot(board.tft, FREQSWEEP_START, FREQSWEEP_END);
      board.tft.waitUpdateAsyncComplete();
      board.tft.updateScreen();
    }
  }
}

void runSweepWrapper(float _) {
  float amp = getLCRAmplitude();
  float freq = getLCRFrequency();
  
  runSweep(FREQSWEEP_START, FREQSWEEP_END, num_points);

  setLCRAmplitude(amp);
  setLCRFrequency(freq);
  loadCalibrationPoint(freq);
}
