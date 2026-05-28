#include "display.h"
#include "constants.h"
#include "fsm.h"
#include "board.h"


long unsigned int prev_refresh_time = 0;

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

void FloatDisplay::configSettings(uint digits, int min_exp, float max_value, String unit) {
  this->digits = digits;
  this->min_exp = min_exp;
  this->max_value = max_value;
  this->unit = unit;
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
    case -12:
      prefix = 'p';
      break;
    case -9:
      prefix = 'n';
      break;
    case -6:
      prefix = 'u';
      break;
    case -3:
      prefix = 'm';
      break;
    case 0:
      prefix = ' ';
      break;
    case 3:
      prefix = 'k';
      break;
    case 6:
      prefix = 'M';
      break;
    case 9:
      prefix = 'G';
      break;
    default:
      overflow = true;
      break;
    }

  int leading_digits = max(floor(log10(coeff)) + 1, 1);
  int decimal_digits = digits - leading_digits - 1;
  text = String(coeff, decimal_digits) + " " + prefix + unit;
  
  Serial.println(text);
}





void drawAll(bool force_update) {
  
  if (millis() - prev_refresh_time < DISP_REFRESH_TIME && !force_update) return;

  //This display function should only ever be called after checking this first, but just in case;
  if (board.tft.asyncUpdateActive()) return;
  
  board.tft.fillScreen(ILI9341_BLACK);

  switch(current_state) {
    
    case RUNNING:
      current_menu->drawMenu(board.tft);
      //print lcr data
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
