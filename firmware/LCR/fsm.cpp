#include "fsm.h"
#include "constants.h"
#include "calibration.h"
#include "board.h"
#include "codec.h"
#include "lcr_func.h"



MenuBar calibration_menu;
MenuBar main_menu_1;
MenuBar main_menu_2;
MenuBar freqsel_menu;
MenuBar ampsel_menu;
MenuBar modesel_menu;

MenuBar *current_menu;

Selector freq_selector;
Selector amp_selector;
Selector *current_selector;

ModeSelector mode_selector;

SYSTEM_STATE current_state;


void switchMainMenuPage(float _ = 0) {
  if (current_menu == &main_menu_1) current_menu = &main_menu_2;
  else current_menu = &main_menu_1;
}

void switchToCalMenu(float _ = 0) {
  current_menu = &calibration_menu;
  current_state = CALIBRATION;
}

void switchToMainMenu(float _ = 0) {
  //Do not go to main LCR page if cal data is not loaded
  if (num_cal_points == 0) return;
  
  current_menu = &main_menu_1;
  current_state = RUNNING;
}

void switchToFreqSelector(float _) {
  current_state = FREQ_INPUT;
  current_menu = &freqsel_menu;
  current_selector = &freq_selector;
}

void exitFreqSelector() {
  char buf[16];
  float val = freq_selector.getValue();
  setLCRFrequency(val);

  if (val > 1000.0) {
    val /= 1000.0;
    if (val > 99.9) snprintf(buf, sizeof(buf), "%.0fkHz", val);
    if (val > 9.99) snprintf(buf, sizeof(buf), "%.1fkHz", val);
    else            snprintf(buf, sizeof(buf), "%.2fkHz", val);
  } else {
    snprintf(buf, sizeof(buf), "%.0fHz", val);
  }

  main_menu_1.setSelectedText(buf, 0);
  switchToMainMenu();
}

void exitAmpSelector() {
  char buf[16];
  float val = amp_selector.getValue();
  setLCRAmplitude(val);

 
  snprintf(buf, sizeof(buf), "%.1fV", val);
 

  main_menu_2.setSelectedText(buf, 1);
  switchToMainMenu();
}

void exitModeSelector() {
  switchToMainMenu();
}

void switchToAmpSelector(float _) {
  current_state = AMP_INPUT;
  current_menu = &ampsel_menu;
  current_selector = &amp_selector;
}

void switchToModeSelector(float _) {
  current_state = MODE_INPUT;
  current_menu = &modesel_menu;
}

void setSelectorIncrement(float f) {
  current_selector->setIncrement(f);
}

void setModeSelector(float f) {
  mode_selector.setSelector(round(f));
}

void initCalMenu() {
  calibration_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);
  
  calibration_menu.addCategory("Probe", nullptr, 0, false, false);
  calibration_menu.addItem("Quick", &calibrateProbes_Point, 0.0f);
  calibration_menu.addItem("Full", &calibrateProbes, 0.0f);

  calibration_menu.addCategory("All", nullptr, 0, false, false);
  calibration_menu.addItem("Quick", &calibrateAll_Point, 0.0f);
  calibration_menu.addItem("Full", &calibrateAll, 0.0f);
  
  calibration_menu.addCategory("Save", nullptr, 0, false);
  calibration_menu.addItem("Confirm", &saveCalibrationWrapper, 0.0f);

  calibration_menu.addCategory("Home", &switchToMainMenu, 0, false);

}

void initMainMenu1() {
  main_menu_1.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  main_menu_1.addCategory("Freq");
  main_menu_1.addItem("100Hz", &setLCRFrequency, 100.0f);
  main_menu_1.addItem("1kHz", &setLCRFrequency, 1000.0f);
  main_menu_1.addItem("10kHz", &setLCRFrequency, 10000.0f);
  main_menu_1.addItem("75kHz", &setLCRFrequency, 75000.0f);
  main_menu_1.addItem("Custom", &switchToFreqSelector);
  main_menu_1.executeItem(main_menu_1.getCategoriesCount()-1, 1);

  main_menu_1.addCategory("Mode");
  main_menu_1.addItem("Auto", &setLCRParams, 0.0f);
  main_menu_1.addItem("Cs+Rs", &setLCRParams, 1.0f);
  main_menu_1.addItem("Ls+Rs", &setLCRParams, 2.0f);
  main_menu_1.addItem("Cp+Rp", &setLCRParams, 3.0f);
  main_menu_1.addItem("Custom", &switchToModeSelector, 0.0f);
  main_menu_1.executeItem(main_menu_1.getCategoriesCount()-1, 0);

  main_menu_1.addCategory("Cal", &switchToCalMenu, 0, false);

  main_menu_1.addCategory("Page 2", &switchMainMenuPage, 0, false);
}

void initMainMenu2() {
  main_menu_2.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  main_menu_2.addCategory("Filt");
  main_menu_2.addItem("1", nullptr, 100.0f);
  main_menu_2.addItem("4", nullptr, 1000.0f);
  main_menu_2.addItem("16", nullptr, 10000.0f);
  main_menu_2.addItem("64", nullptr, 75000.0f);
  main_menu_2.executeItem(main_menu_2.getCategoriesCount()-1, 0);

  main_menu_2.addCategory("Ampl");
  main_menu_2.addItem("0.5V", &setLCRAmplitude, 0.5f);
  main_menu_2.addItem("1.0V", &setLCRAmplitude, 1.0f);
  main_menu_2.addItem("2.0V", &setLCRAmplitude, 2.0f);
  main_menu_2.addItem("3.5V", &setLCRAmplitude, 3.5f);
  main_menu_2.addItem("Custom", &switchToAmpSelector);
  main_menu_2.executeItem(main_menu_2.getCategoriesCount()-1, 3);

  main_menu_2.addCategory("Other");
  main_menu_2.addItem("0.1V", nullptr, 0.1f);
  main_menu_2.addItem("0.5V", nullptr, 0.5f);
  main_menu_2.addItem("1.0V", nullptr, 1.0f);
  main_menu_2.addItem("2.0V", nullptr, 2.0f);
  main_menu_2.executeItem(main_menu_2.getCategoriesCount()-1, 3);

  main_menu_2.addCategory("Page 1", &switchMainMenuPage, 0, false);
}

void initFreqSelMenu() {
  freqsel_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  freqsel_menu.addCategory("10kHz", &setSelectorIncrement, 10000, false);
  freqsel_menu.addCategory("1kHz", &setSelectorIncrement, 1000, false);
  freqsel_menu.addCategory("100Hz", &setSelectorIncrement, 100, false);
  freqsel_menu.addCategory("10Hz", &setSelectorIncrement, 10, false);
}

void initAmpSelMenu() {
  ampsel_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  ampsel_menu.addCategory("", nullptr);
  ampsel_menu.addCategory("1V", &setSelectorIncrement, 1.0, false);
  ampsel_menu.addCategory("100mV", &setSelectorIncrement, 0.1, false);
  ampsel_menu.addCategory("10mV", &setSelectorIncrement, 0.01, false);
}

void initModeSelMenu() {
  modesel_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  modesel_menu.addCategory("Primary", &setModeSelector, 0.0, false);
  modesel_menu.addCategory("Secondary", &setModeSelector, 1.0, false);
}

void initSystem() {

  board.tft.fillScreen(ILI9341_BLACK);
  board.tft.updateScreenAsync();

  uint8_t points_loaded = loadCalibration();
  if (points_loaded == 0) {
    calibrateAll();
  }
  current_state = RUNNING;
  current_menu = &main_menu_1;
  current_selector = &freq_selector;
  
  initCalMenu();
  initMainMenu1();
  initMainMenu2();
  initFreqSelMenu();
  initAmpSelMenu();
  initModeSelMenu();
  freq_selector.init(100, 90000, 100);
  freq_selector.setIncrement(100);

  amp_selector.init(.05, 3.5, 1);
  amp_selector.setIncrement(.1);

  mode_selector.init(lcr_param_lookup, LCR_FUNC_NUM);

}

void runMenuInterface() {
  uint8_t res1 = 0;
  uint8_t res2 = 0;
  uint8_t res;
  
  if (board.tsPressed()) {
    res1 = current_menu->processTouch(board.ts_x, board.ts_y);
  } 

  if (board.up_button.pressed()) {
    res2 = current_menu->moveUp();
  } else if (board.down_button.pressed()) {
    res2 = current_menu->moveDown();
  } else if (board.enter_button.pressed()) {
    res2 = current_menu->enter();
  } else if (board.select_button_0.pressed()) {
    res2 = current_menu->toggleCategory(0);
  } else if (board.select_button_1.pressed()) {
    res2 = current_menu->toggleCategory(1);
  } else if (board.select_button_2.pressed()) {
    res2 = current_menu->toggleCategory(2);
  } else if (board.select_button_3.pressed()) {
    res2 = current_menu->toggleCategory(3);
  }

  res = max(res1, res2);
  
  if (res == 1) {
    board.buzzer.setBuzzer(1, 10, 1);
  } else if (res == 2) {
    board.buzzer.setBuzzer(3, 15, 55);
  }
}

void runSelectorInterface() {
  uint8_t res1 = 0;
  uint8_t res2 = 0;
  uint8_t res;

  bool exit_selector = false;
  
  if (board.tsPressed()) {
    res1 = current_menu->processTouch(board.ts_x, board.ts_y);
  } 
  
  if (board.up_button.pressed() || board.up_button.process_hold()) {
    current_selector->incrementUp();
    res2 = 1;
  } else if (board.down_button.pressed() || board.down_button.process_hold()) {
    current_selector->incrementDown();
    res2 = 1;
  } else if (board.enter_button.pressed()) {
    res2 = 2;
    exit_selector = true;
  } else if (board.select_button_0.pressed()) {
    res2 = current_menu->toggleCategory(0);
  } else if (board.select_button_1.pressed()) {
    res2 = current_menu->toggleCategory(1);
  } else if (board.select_button_2.pressed()) {
    res2 = current_menu->toggleCategory(2);
  } else if (board.select_button_3.pressed()) {
    res2 = current_menu->toggleCategory(3);
  }

  res = max(res1, res2);
  
  if (res == 1) {
    board.buzzer.setBuzzer(1, 10, 1);
  } else if (res == 2) {
    board.buzzer.setBuzzer(3, 15, 55);
  }
  
  if (exit_selector) {
    if (current_selector == &freq_selector) {
      exitFreqSelector();
    } else if (current_selector == &amp_selector) {
      exitAmpSelector();
    }
  }

}


void runModeSelInterface() {
  uint8_t res1 = 0;
  uint8_t res2 = 0;
  uint8_t res;

  bool exit_selector = false;
  
  if (board.tsPressed()) {
    res1 = current_menu->processTouch(board.ts_x, board.ts_y);
  } 

  if (board.up_button.pressed() || board.up_button.process_hold()) {
    mode_selector.incrementModeUp();
    res2 = 1;
    Serial.println(mode_selector.getPrimary().label);
  } else if (board.down_button.pressed() || board.down_button.process_hold()) {
    mode_selector.incrementModeDown();
    res2 = 1;
    Serial.println(mode_selector.getPrimary().label);
  } else if (board.enter_button.pressed()) {
    //res2 = current_menu->enter();
    res2 = 2;
    exit_selector = true;
  } else if (board.select_button_0.pressed()) {
    res2 = current_menu->toggleCategory(0);
  } else if (board.select_button_1.pressed()) {
    res2 = current_menu->toggleCategory(0);
  } else if (board.select_button_2.pressed()) {
    res2 = current_menu->toggleCategory(1);
  } else if (board.select_button_3.pressed()) {
    res2 = current_menu->toggleCategory(1);
  }

  res = max(res1, res2);

  if (res == 1) {
    board.buzzer.setBuzzer(1, 10, 1);
  } else if (res == 2) {
    board.buzzer.setBuzzer(3, 15, 55);
  }

  if (exit_selector) exitModeSelector();
 
}


void runSystem() {

  switch(current_state) {
    
    case RUNNING:
      runLCR();
      runMenuInterface();
      break;

    case CALIBRATION:
      runMenuInterface();
      break;

    case FREQ_INPUT:
      runSelectorInterface();
      break;

    case AMP_INPUT:
      runSelectorInterface();
      break;

    case MODE_INPUT:
      runModeSelInterface();
      break;
      
    default:
      break;
    
  }
  
  board.buzzer.runBuzzer(micros());
}
