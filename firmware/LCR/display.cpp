#include "display.h"
#include "constants.h"
#include "fsm.h"
#include "board.h"
#include "lcr_func.h"
#include "calibration.h"
#include "LCR_Fonts/Consolas_Bold20pt7b.h"
#include "LCR_Fonts/FreeMono9pt7b.h"
#include "LCR_Fonts/Font5x7FixedMono.h"


LogPrint logger;

long unsigned int prev_refresh_time = 0;

FloatDisplay lcr_primary;
FloatDisplay lcr_secondary;

void FloatDisplay::init(uint xpos, uint ypos, bool hysteresis) {
  this->xpos = xpos;
  this->ypos = ypos;
  this->hysteresis = hysteresis;
  exponent = 0;
  forced_exponent = false;
  prev_leading_digits = -1;
}

void FloatDisplay::init(uint xpos, uint ypos, int exponent) {
  this->xpos = xpos;
  this->ypos = ypos;
  hysteresis = false;
  this->exponent = exponent;
  forced_exponent = true;
  exponent = 0;
  prev_leading_digits = -1;
}

void FloatDisplay::configSettings(uint digits, int min_exp, int min_res, float max_value, const char *unit, const char *label) {
  this->digits = digits;
  this->min_exp = min_exp;
  this->max_value = max_value;
  this->unit = unit;
  this->label = label;
  this->min_resolution = min_res;
}

void FloatDisplay::configSettings(lcr_param_t &params) {
  this->digits = DISP_DIGITS;
  this->min_exp = params.min_exp;
  this->max_value = 10e6;
  this->unit = params.unit;
  this->label = params.label;
  this->min_resolution = params.min_res;
}

void FloatDisplay::updateValue(float value) {
  int exponent;

  if (forced_exponent) {
    exponent = this->exponent;
  } else {
    exponent = 3 * floor(log10(value) / 3);
  }

  exponent = max(min_exp, exponent);
  float coeff = value * pow(10, -1 * exponent);

  if (hysteresis && exponent != this->exponent) {
    if (exponent > this->exponent && coeff < DISP_FLOAT_RANGE_UP && exponent != min_exp) {
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
    case 0:   prefix = ""; break;
    case 3:   prefix = 'k'; break;
    case 6:   prefix = 'M'; break;
    case 9:   prefix = 'G'; break;
    default:  overflow = true; break;
  }

  int leading_digits = max(floor(log10(coeff)) + 1, 1);

  if (DISP_FLOAT_DECIMAL_HYST && prev_leading_digits == leading_digits + 1 && value / pow(10, floor(log10(value)) + 1) > DISP_FLOAT_RANGE_DOWN) {
    leading_digits += 1;
    int decimal_digits = digits - leading_digits - 1;
    text = String(coeff, decimal_digits);
    
  } else {

    //Determine required number of trailing decimal digits
    int decimal_digits = min(digits - leading_digits - 1, exponent - min_resolution);
    decimal_digits = max(decimal_digits, 1);
    
    text = String(coeff, decimal_digits);
    if (text.length() > digits) {
      //value rounded up, chop off last digit
      text = text.substring(0, digits);
    }
  }

  //Append 0's to front of number to reach desired number of digits
  while (text.length() < digits) {
    text = "0" + text;
  }
    
  prev_leading_digits = leading_digits;

  text = String(label) + ": " + text + " " + prefix + unit;

  Serial.println(text);
}

void FloatDisplay::draw(ILI9341_t3n &tft) {
  tft.setTextColor(ILI9341_RED);
  tft.setFont(&Consolas_Bold20pt7b);
  tft.setTextSize(1);

  tft.setCursor(xpos, ypos);
  tft.print(text);

}

void LogPrint::init(ILI9341_t3n &tft, bool serial_logging, const GFXfont *font) {
  this->tft = &tft;
  this->serial_logging = serial_logging;
  this->font = font;
}

void LogPrint::setCursor(uint16_t x, uint16_t y, uint16_t line_advance) {
  this->x = x;
  this->y = y;
  this->line_advance = line_advance;

  tft->setTextSize(1);
  tft->setTextColor(ILI9341_WHITE);
  tft->setFont(font);
  tft->setCursor(x, y);
}

void LogPrint::print(float f) {
  print(String(f, 3));
}

void LogPrint::print(Complex c) {
  tft->print(c);
  if (serial_logging) Serial.print(c);
}

void LogPrint::print(String s) {
  tft->print(s);
  if (serial_logging) Serial.print(s);
}

void LogPrint::println(float f) {
  println(String(f, 3));
}

void LogPrint::println() {
  println("");
}

void LogPrint::println(Complex c) {
  tft->print(c);
  if (serial_logging) Serial.println(c);
  y += line_advance; 
  tft->setCursor(x, y);
}

void LogPrint::println(String s) {
  tft->print(s);
  if (serial_logging) Serial.println(s);
  y += line_advance; 
  tft->setCursor(x, y);
}

void LogPrint::println_large(String s) {
  tft->setTextSize(2);
  tft->print(s);
  if (serial_logging) Serial.println(s);
  y += 2*line_advance; 
  tft->setCursor(x, y);
  tft->setTextSize(1);
}

void userPromptText(String s) {
  board.tft.setTextColor(ILI9341_RED);
  board.tft.setFont(&FreeMono9pt7b);
  int16_t x1, y1;
  uint16_t w, h;
  
  board.tft.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  int16_t text_x = (SCREEN_WIDTH - w) / 2;
  int16_t text_y = SCREEN_HEIGHT - 60 - y1;
  
  board.tft.setCursor(text_x, text_y);
  board.tft.print(s);

  board.tft.getTextBounds("Press Enter or Touch", 0, 0, &x1, &y1, &w, &h);
  text_x = (SCREEN_WIDTH - w) / 2;
  board.tft.setCursor(text_x, text_y + 20);
  
  board.tft.print("Press Enter or Touch");
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
}


void initDraw() {
  lcr_primary.init(20, 100, true);
  lcr_secondary.init(20, 140, true);
  logger.init(board.tft, false, &Font5x7FixedMono);
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

  switch (v_pga) {
    case PGA_GAIN_1:    v_pga_text += "1x";   break;
    case PGA_GAIN_5:    v_pga_text += "5x";   break;
    case PGA_GAIN_25:   v_pga_text += "25x";  break;
    case PGA_GAIN_100:  v_pga_text += "100x"; break;
    default:            v_pga_text += "err";  break;
  }

  switch (i_pga) {
    case PGA_GAIN_1:    i_pga_text += "1x";   break;
    case PGA_GAIN_5:    i_pga_text += "5x";   break;
    case PGA_GAIN_25:   i_pga_text += "25x";  break;
    case PGA_GAIN_100:  i_pga_text += "100x"; break;
    default:            i_pga_text += "err";  break;
  }

  switch (range) {
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

String floatToExp(float val) {
  if (val == 0) return "0e0";
  bool neg = val < 0;
  val = abs(val);

  if (val < 1e-3) return "0e0";
  if (val > R_OVERFLOW) return "ovf";

  int val_exp = floor(log10(val));
  int coeff = round(val / pow(10, val_exp));
  if (coeff == 10) {
    coeff = 1;
    val_exp += 1;
  }

  return String(coeff) + "e" + String(val_exp);

  /*
    if (neg) {
    return "-" + String(coeff) + "e" + String(val_exp);
    } else {
    return "+" + String(coeff) + "e" + String(val_exp);
    }
  */
}

void drawImpedance() {
  float Rs = resistance_lcr_value;
  float Xs = reactance_lcr_value;

  String text = "Z=";

  if (Rs < 0) text += "-";

  text += floatToExp(Rs);
  if (Xs < 0) text += "-j";
  else text += "+j";

  text += floatToExp(Xs);
  text += " Ohm";

  board.tft.setTextColor(ILI9341_WHITE);
  board.tft.setFont(&FreeMono9pt7b);
  board.tft.setTextSize(1);

  board.tft.setCursor(10, 70);
  board.tft.print(text);
}

void drawAll(bool force_update) {

  if (millis() - prev_refresh_time < DISP_REFRESH_TIME && !force_update) return;

  //This display function should only ever be called after checking this first, but just in case;
  if (board.tft.asyncUpdateActive()) return;

  board.tft.fillScreen(ILI9341_BLACK);

  switch (current_state) {

    case RUNNING:
      drawLCRReadings();
      drawCurrentRanges();
      drawImpedance();
      current_menu->drawMenu(board.tft);
      break;

    case CALIBRATION:
      printCalibrationPoint(calibration_data);
      current_menu->drawMenu(board.tft);
      //print cal data
      break;

    default:
      break;

  }

  board.tft.updateScreenAsync();

  prev_refresh_time = millis();
}
