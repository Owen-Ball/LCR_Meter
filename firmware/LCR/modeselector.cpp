#include "modeselector.h"
#include "display.h"
#include "LCR_Fonts/Consolas_Bold20pt7b.h"



/*
class ModeSelector {
  public:
    void init(lcr_param_t *mode_array, uint8_t mode_count);
    void setSelector(uint8_t index);
    void incrementModeUp();
    void incrementModeDown();
    lcr_param_t getPrimary();
    lcr_param_t getSecondary();
    void draw(ILI9341_t3n &tft);
  private:
    uint8_t current selector;
    uint8_t primary_index;
    uint8_t secondary_index;
    lcr_param_t mode_array[16];
    uint8_t mode_count;
};

*/



void ModeSelector::init(lcr_param_t **mode_array, uint8_t mode_count) {
  this->mode_count = mode_count;
  this->mode_array = mode_array;
  current_selector = 0;
  primary_index = 0;
  secondary_index = 0;
}

void ModeSelector::setSelector(uint8_t index) {
  if (index > 1) index = 1;
  current_selector = index;
}

void ModeSelector::incrementModeUp() {
  if (current_selector == 0) {
    
    primary_index += 1;
    if (primary_index >= mode_count) primary_index = 0;

  } else {
    
    secondary_index += 1;
    if (secondary_index >= mode_count) secondary_index = 0;

  }
}

void ModeSelector::incrementModeDown() {
  if (current_selector == 0) {
   
    if (primary_index == 0) primary_index = mode_count - 1;
    else primary_index -= 1;

  } else {
    
    if (secondary_index == 0) secondary_index = mode_count - 1;
    else secondary_index -= 1;

  }

}

lcr_param_t ModeSelector::getPrimary() {
    return *mode_array[primary_index];
}

lcr_param_t ModeSelector::getSecondary() {
    return *mode_array[secondary_index];
}

String ModeSelector::getMenuText() {
  String text = "";

  if (getPrimary().label[0] == ' ') {
    text += getPrimary().label[1];
  } else {
    text += getPrimary().label;
  }
  
  text += "+";
  
  if (getSecondary().label[0] == ' ') {
    text += getSecondary().label[1];
  } else {
    text += getSecondary().label;
  }

  return text;
}
void ModeSelector::draw(ILI9341_t3n &tft) {
  tft.setTextColor(ILI9341_RED);
  tft.setFont(&Consolas_Bold20pt7b);
  tft.setTextSize(1);

  String text = "";

  
  if (getPrimary().label[0] == ' ') {
    text += " ";
    text += getPrimary().label[1];
  } else {
    text += getPrimary().label;
  }
  
  text += " + ";
  
  if (getSecondary().label[0] == ' ') {
    text += getSecondary().label[1];
  } else {
    text += getSecondary().label;
  }

  String underline_text;

  if (current_selector == 0) {
    underline_text = "__      ";
  } else {
    underline_text = "     __ ";
  }

  int16_t text_x = getCenteredXPos(underline_text);
  
  tft.setCursor(text_x, 105);
  tft.println(text);
  tft.setCursor(text_x, 105 + 6);
  tft.println(underline_text);

}
