#include "display.h"
#include "constants.h"
#include "fsm.h"
#include "board.h"
#include "lcr_func.h"
#include "LCR_Fonts/FreeMonoBold18pt7b.h"
#include "LCR_Fonts/FreeMono9pt7b.h"


long unsigned int prev_refresh_time = 0;

FloatDisplay lcr_primary;
FloatDisplay lcr_secondary;

void FloatDisplay::init(uint xpos, uint ypos, bool hysteresis) {
  this->xpos = xpos;
  this->ypos = ypos;
  this->hysteresis = hysteresis;
  exponent = 0;
  forced_exponent = false;
}

void FloatDisplay::init(uint xpos, uint ypos, int exponent) {
  this->xpos = xpos;
  this->ypos = ypos;
  hysteresis = false;
  this->exponent = exponent;
  forced_exponent = true;
  exponent = 0;
}

void FloatDisplay::configSettings(uint digits, int min_exp, float max_value, const char *unit) {
  this->digits = digits;
  this->min_exp = min_exp;
  this->max_value = max_value;
  this->unit = unit;
}

void FloatDisplay::configSettings(lcr_param_t &params) {
  this->digits = DISP_DIGITS;
  this->min_exp = params.resolution;
  this->max_value = 10e6;
  this->unit = params.unit;
}

void FloatDisplay::updateValue(float value) {
  int exponent;
  
  if (forced_exponent) {
    exponent = this->exponent;
  } else {
    exponent = 3*floor(log10(value)/3);
  }
  exponent = max(min_exp, exponent);
  float coeff = value * pow(10, -1*exponent);

  if (hysteresis && exponent != this->exponent) {
    if (exponent > this->exponent && coeff < DISP_FLOAT_RANGE_UP) {
      exponent -= 3;
      coeff *= 1000.0;
    }
    if (exponent < this->exponent && coeff > DISP_FLOAT_RANGE_DOWN * 1000.0) {
      exponent += 3;
      coeff /= 1000.0;
    }
  }

  this->coeff = coeff;
  this->exponent = exponent;

  switch (exponent) {
    case -15: prefix = 'a'; break;
    case -12: prefix = 'p'; break;
    case -9:  prefix = 'n'; break;
    case -6:  prefix = 'u'; break;
    case -3:  prefix = 'm'; break;
    case 0:   prefix = ' '; break;
    case 3:   prefix = 'k'; break;
    case 6:   prefix = 'M'; break;
    case 9:   prefix = 'G'; break;
    default:  overflow = true; break;
  }

  int leading_digits = max(floor(log10(coeff)) + 1, 1);
  int decimal_digits = digits - leading_digits - 1;
  text = String(coeff, decimal_digits) + " " + prefix + unit;
  
  Serial.println(text);
}

void FloatDisplay::draw(ILI9341_t3n &tft) {
  tft.setTextColor(ILI9341_RED);   // set color
  tft.setFont(&FreeMonoBold18pt7b);
  tft.setTextSize(1);

  tft.setCursor(xpos, ypos);
  tft.print(text);

}

void initDraw() {
  lcr_primary.init(50, 100, true);
  lcr_secondary.init(50, 140, true);
}


void drawLCRReadings() {
  lcr_primary.configSettings(primary_lcr_param);
  lcr_primary.updateValue(primary_lcr_value);  
  lcr_secondary.configSettings(secondary_lcr_param);
  lcr_secondary.updateValue(secondary_lcr_value);

  //Serial.print(codecReadings.v_peak);
  //Serial.print(" ");
  //Serial.println(codecReadings.i_peak);

  lcr_primary.draw(board.tft);
  lcr_secondary.draw(board.tft);
}

void drawCurrentRanges() {
  uint8_t v_pga = board.getPGAGainV();
  uint8_t i_pga = board.getPGAGainI();
  uint8_t range = board.getLCRRange();

  String v_pga_text = "V PGA: ";
  String i_pga_text = "I PGA: ";
  String range_text = "Range: ";
  
  switch(v_pga) {
    case PGA_GAIN_1:    v_pga_text += "1x";   break;
    case PGA_GAIN_5:    v_pga_text += "5x";   break;
    case PGA_GAIN_25:   v_pga_text += "25x";  break;
    case PGA_GAIN_100:  v_pga_text += "100x"; break;
    default:            v_pga_text += "err";  break;
  }

  switch(i_pga) {
    case PGA_GAIN_1:    i_pga_text += "1x";   break;
    case PGA_GAIN_5:    i_pga_text += "5x";   break;
    case PGA_GAIN_25:   i_pga_text += "25x";  break;
    case PGA_GAIN_100:  i_pga_text += "100x"; break;
    default:            i_pga_text += "err";  break;
  }

  switch(range) {
    case LCR_RANGE_100:   range_text += "100O";  break;
    case LCR_RANGE_1K:    range_text += "1kO";   break;
    case LCR_RANGE_10K:   range_text += "10kO";  break;
    case LCR_RANGE_100K:  range_text += "100kO"; break;
    default:              range_text += "err";   break;
  }

  board.tft.setTextColor(ILI9341_WHITE);
  board.tft.setFont(&FreeMono9pt7b);
  board.tft.setTextSize(1);
  
  board.tft.setCursor(10, 10);
  board.tft.print(v_pga_text);
  board.tft.setCursor(10, 30);
  board.tft.print(i_pga_text);
  board.tft.setCursor(10, 50);
  board.tft.print(range_text);
  
}

void drawAll(bool force_update) {
  
  if (millis() - prev_refresh_time < DISP_REFRESH_TIME && !force_update) return;

  //This display function should only ever be called after checking this first, but just in case;
  if (board.tft.asyncUpdateActive()) return;
  
  board.tft.fillScreen(ILI9341_BLACK);

  switch(current_state) {
    
    case RUNNING:
      drawLCRReadings();
      drawCurrentRanges();
      current_menu->drawMenu(board.tft);
      break;

    case CALIBRATION:
      current_menu->drawMenu(board.tft);
      //print cal data
      break;
      
    default:
      break;
    
  }
  
  board.tft.updateScreenAsync();

  prev_refresh_time = millis();
}
