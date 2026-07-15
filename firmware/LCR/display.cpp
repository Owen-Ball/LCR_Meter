#include "display.h"
#include "constants.h"
#include "fsm.h"
#include "board.h"
#include "lcr_func.h"
#include "calibration.h"
#include "selector.h"
#include "LCR_Fonts/Consolas_Bold20pt7b.h"
#include "LCR_Fonts/FreeMono9pt7b.h"
#include "LCR_Fonts/Font5x7FixedMono.h"
#include "images.h"
#include "polarplot.h"
#include "sweep.h"


LogPrint logger;

long unsigned int prev_refresh_time = 0;

FloatDisplay lcr_primary;
FloatDisplay lcr_secondary;

FloatDisplay freq_sel_display;
FloatDisplay amp_sel_display;

void FloatDisplay::init(uint xpos, uint ypos, bool hysteresis) {
  this->xpos = xpos;
  this->ypos = ypos;
  this->hysteresis = hysteresis;
  exponent = 0;
  forced_exponent = false;
  prev_leading_digits = -1;
  underline = false;
  underlined_digit = 0;
}

void FloatDisplay::init(uint xpos, uint ypos, int exponent) {
  this->xpos = xpos;
  this->ypos = ypos;
  hysteresis = false;
  this->exponent = exponent;
  forced_exponent = true;
  exponent = 0;
  prev_leading_digits = -1;
  underline = false;
  underlined_digit = 0;
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

  if (value < 0) neg = true;
  else neg = false;
  
  value = abs(value);

  if (value == INFINITY) {
    this->exponent = -21;
    text = String(label) + ": Ovrflw " + unit;
    underline_text = "";
    return;
  }
  
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
    int decimal_digits = min(int(digits) - leading_digits - 1, exponent - min_resolution);
    decimal_digits = max(decimal_digits, 0);
    
    text = String(coeff, decimal_digits);
    if (text.length() > digits) {
      //value rounded up, chop off last digit
      text = text.substring(0, digits);
    }
  }

  int leading_spaces = exponent + leading_digits - underlined_digit - 1;

  if (underlined_digit < exponent) {
    leading_spaces += 1;
  }
  
  //Append 0's to front of number to reach desired number of digits
  while (text.length() < digits) {
    text = "0" + text;
    leading_spaces += 1;
  }
  
  prev_leading_digits = leading_digits;

  underline_text = "";
  if (leading_spaces >= 0) {
    for (int i = 0; i < leading_spaces; i++) {
      underline_text = " " + underline_text;
    }
    underline_text += "_";
    underline_text = underline_text.substring(0, text.length());
    
    for (int i = 0; i < 2 + int(String(label).length()); i++) {
      underline_text = " " + underline_text;
    }
  }

  if (neg) text = String(label) + ":-" + text + " " + prefix + unit;
  else     text = String(label) + ": " + text + " " + prefix + unit;
  
}

void FloatDisplay::underlineDigit(int digit) {
  underline = true;
  underlined_digit = digit;
}

void FloatDisplay::draw(ILI9341_t3n &tft) {
  tft.setTextColor(ILI9341_RED);
  tft.setFont(&Consolas_Bold20pt7b);
  tft.setTextSize(1);

  tft.setCursor(xpos, ypos);
  tft.println(text);
  if (underline) {
    tft.setCursor(xpos, ypos);
    tft.print(underline_text);
  }

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

int16_t getCenteredXPos(String s, int16_t x_center) {
  int16_t x1, y1;
  uint16_t w, h;
  
  board.tft.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  return x_center - w / 2;
}

void userPromptText(String s) {
  board.tft.setTextColor(ILI9341_RED);
  board.tft.setFont(&FreeMono9pt7b);
 
  int16_t text_x = getCenteredXPos(s);
  int16_t text_y = SCREEN_HEIGHT - 80;
  
  board.tft.setCursor(text_x, text_y);
  board.tft.print(s);

  text_x = getCenteredXPos("Press Enter or Touch");
  board.tft.setCursor(text_x, text_y + 25);
  
  board.tft.print("Press Enter or Touch");
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
}

void drawScreenTitle(String title) {
  board.tft.setTextColor(ILI9341_WHITE);
  board.tft.setFont(&Consolas_Bold20pt7b);
  board.tft.setTextSize(1);

  int16_t text_x = getCenteredXPos(title);
  int16_t text_y = 20;
  
  board.tft.setCursor(text_x, text_y);
  board.tft.print(title);
}

void initDraw() {
  lcr_primary.init(23, 105, true);
  lcr_secondary.init(23, 145, true);
  freq_sel_display.init(40, 100, 0);
  freq_sel_display.configSettings(5, 0, 0, 100000, "Hz", "F");
  amp_sel_display.init(40, 100, -3);
  amp_sel_display.configSettings(4, -3, -3, 100000, "V", "A");
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
    case LCR_RANGE_100:   range_text += "100@";  break;
    case LCR_RANGE_1K:    range_text += "1k@";   break;
    case LCR_RANGE_10K:   range_text += "10k@";  break;
    case LCR_RANGE_100K:  range_text += "100k@"; break;
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
  if (val > Z_OVERFLOW) return "ovf";

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
  text += " @";

  board.tft.setTextColor(ILI9341_WHITE);
  board.tft.setFont(&FreeMono9pt7b);
  board.tft.setTextSize(1);

  board.tft.setCursor(10, 70);
  board.tft.print(text);
}

void drawFreqSelector() {
  int digit = round(log10(freq_selector.getIncrement()));
  freq_sel_display.underlineDigit(digit);
  freq_sel_display.updateValue(freq_selector.getValue());
  freq_sel_display.draw(board.tft);
}

void drawAmpSelector() {
  int digit = round(log10(amp_selector.getIncrement()));
  amp_sel_display.underlineDigit(digit);
  amp_sel_display.updateValue(amp_selector.getValue());
  amp_sel_display.draw(board.tft);
}

void drawProbes() {
  if (current_probes == CLIP_PROBES) {
    board.tft.drawBitmap(PROBE_BMP_X_POS, PROBE_BMP_Y_POS, clip_bmp, PROBE_BMP_WIDTH, PROBE_BMP_HEIGHT, 0xef5d);
  } else if (current_probes == TWEEZER_PROBES) {
    board.tft.drawBitmap(PROBE_BMP_X_POS, PROBE_BMP_Y_POS, tweezers_bmp, PROBE_BMP_WIDTH, PROBE_BMP_HEIGHT, 0xef5d);
  }
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
      drawProbes();
      polarPlotZ(Complex(resistance_lcr_value, reactance_lcr_value));
      break;

    case CALIBRATION:
      printCalibrationPoint(calibration_data);
      current_menu->drawMenu(board.tft);
      drawProbes();
      break;

    case FREQ_INPUT:
      current_menu->drawMenu(board.tft);
      drawFreqSelector();
      break;

    case AMP_INPUT:
      current_menu->drawMenu(board.tft);
      drawAmpSelector();
      break;
      
    case MODE_INPUT:
      current_menu->drawMenu(board.tft);
      drawScreenTitle("Set Params");
      mode_selector.draw(board.tft);
      break;

    case FREQ_SWEEP:
      drawSweepWrapper(board.tft);
      current_menu->drawMenu(board.tft);
      break;
      
    default:
      break;

  }

  board.tft.updateScreenAsync();

  prev_refresh_time = millis();
}
