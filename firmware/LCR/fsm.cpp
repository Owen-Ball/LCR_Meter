#include "fsm.h"
#include "constants.h"
#include "calibration.h"
#include "board.h"
#include "codec.h"
#include "lcr_func.h"
#include "images.h"
#include "sweep.h"



MenuBar calibration_menu;
MenuBar main_menu_1;
MenuBar main_menu_2;
MenuBar freqsel_menu;
MenuBar ampsel_menu;
MenuBar modesel_menu;
MenuBar freqsweep_menu;

MenuBar *current_menu;

Selector freq_selector;
Selector amp_selector;
Selector *current_selector;

ModeSelector mode_selector;

SYSTEM_STATE current_state;

bool touch_to_process;


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

void exitFreqSelector(float _ = 0) {
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

void exitAmpSelector(float _ = 0) {
  char buf[16];
  float val = amp_selector.getValue();
  setLCRAmplitude(val);

 
  snprintf(buf, sizeof(buf), "%.1fV", val);
 

  main_menu_2.setSelectedText(buf, 1);
  switchToMainMenu();
  switchMainMenuPage();
}

void exitModeSelector(float _ = 0) {
  char buf[16];
  String disp_text = mode_selector.getMenuText();
  disp_text.toCharArray(buf, sizeof(buf));
 
  main_menu_1.setSelectedText(buf, 1);
  
  primary_lcr_param = mode_selector.getPrimary();
  secondary_lcr_param = mode_selector.getSecondary();
  auto_param = false;
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

void switchToFreqSweep(float _) {
  current_state = FREQ_SWEEP;
  current_menu = &freqsweep_menu;
}

void setSelectorIncrement(float f) {
  current_selector->setIncrement(f);
}

void setModeSelector(float f) {
  mode_selector.setSelector(round(f));
}

void selectTweezers(float _) {
  current_probes = TWEEZER_PROBES;
  loadCalibration();
  resetAverageZ();

  float freq = codecGetFrequency();
  loadCalibrationPoint(freq);
}

void selectClips(float _) {
  current_probes = CLIP_PROBES;
  loadCalibration();
  resetAverageZ();

  float freq = codecGetFrequency();
  loadCalibrationPoint(freq);
}

void initCalMenu() {
  calibration_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);
  
  calibration_menu.addCategory("Probe", nullptr, 0, false, false);
  calibration_menu.addItem("Quick", &calibrateProbes_Point, 0.0f);
  calibration_menu.addItem("Full", &calibrateProbes, 0.0f);
  calibration_menu.addItem("Twzrs", &selectTweezers, 0.0f);
  calibration_menu.addItem("Clips", &selectClips, 0.0f);

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
  main_menu_1.addItem("[]+<Z", &setLCRParams, 3.0f);
  main_menu_1.addItem("Custom", &switchToModeSelector, 0.0f);
  main_menu_1.executeItem(main_menu_1.getCategoriesCount()-1, 0);

  main_menu_1.addCategory("Cal", &switchToCalMenu, 0, false);

  main_menu_1.addCategory("Page 2", &switchMainMenuPage, 0, false);
}

void initMainMenu2() {
  main_menu_2.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  main_menu_2.addCategory("Filt");
  main_menu_2.addItem("1", &setAverageZ, 1.0f);
  main_menu_2.addItem("4", &setAverageZ, 4.0f);
  main_menu_2.addItem("16", &setAverageZ, 16.0f);
  main_menu_2.addItem("64", &setAverageZ, 64.0f);
  main_menu_2.executeItem(main_menu_2.getCategoriesCount()-1, 1);

  main_menu_2.addCategory("Ampl");
  main_menu_2.addItem("0.5V", &setLCRAmplitude, 0.5f);
  main_menu_2.addItem("1.0V", &setLCRAmplitude, 1.0f);
  main_menu_2.addItem("2.0V", &setLCRAmplitude, 2.0f);
  main_menu_2.addItem("3.5V", &setLCRAmplitude, 3.5f);
  main_menu_2.addItem("Custom", &switchToAmpSelector);
  main_menu_2.executeItem(main_menu_2.getCategoriesCount()-1, 3);

  main_menu_2.addCategory("Sweep", &switchToFreqSweep, 0, false);

  main_menu_2.addCategory("Page 1", &switchMainMenuPage, 0, false);
}

void initFreqSelMenu() {
  freqsel_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  freqsel_menu.addCategory("10kHz", &setSelectorIncrement, 10000, false);
  freqsel_menu.addCategory("1kHz", &setSelectorIncrement, 1000, false);
  freqsel_menu.addCategory("100Hz", &setSelectorIncrement, 100, false);
  freqsel_menu.addCategory("Set", &exitFreqSelector, 0.0, false);
}

void initAmpSelMenu() {
  ampsel_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  ampsel_menu.addCategory("1V", &setSelectorIncrement, 1.0, false);
  ampsel_menu.addCategory("100mV", &setSelectorIncrement, 0.1, false);
  ampsel_menu.addCategory("10mV", &setSelectorIncrement, 0.01, false);
  ampsel_menu.addCategory("Set", &exitAmpSelector, 0.0, false);
}

void initModeSelMenu() {
  modesel_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  modesel_menu.addCategory("", nullptr, 0.0, false);
  modesel_menu.addCategory("Prim.", &setModeSelector, 0.0, false);
  modesel_menu.addCategory("Sec.", &setModeSelector, 1.0, false);
  modesel_menu.addCategory("Set", &exitModeSelector, 0.0, false);
}


void initFreqSweepMenu() {
  freqsweep_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  freqsweep_menu.addCategory("Points");
  freqsweep_menu.addItem("20", &setFreqSweepPoints, 20.0f);
  freqsweep_menu.addItem("50", &setFreqSweepPoints, 50.0f);
  freqsweep_menu.addItem("100", &setFreqSweepPoints, 100.0f);
  freqsweep_menu.addItem("200", &setFreqSweepPoints, 200.0f);
  freqsweep_menu.executeItem(freqsweep_menu.getCategoriesCount()-1, 1);

  freqsweep_menu.addCategory("Mode");
  freqsweep_menu.addItem("[]+<Z", &setFreqSweepDisplayMode, 0.0f);
  freqsweep_menu.addItem("R+jX", &setFreqSweepDisplayMode, 1.0f);
  freqsweep_menu.addItem("[]+Q", &setFreqSweepDisplayMode, 2.0f);
  freqsweep_menu.executeItem(freqsweep_menu.getCategoriesCount()-1, 0);

  freqsweep_menu.addCategory("Start", &runSweepWrapper, 0, false);

  freqsweep_menu.addCategory("Home", &switchToMainMenu, 0, false);
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
  initFreqSweepMenu();
  
  freq_selector.init(100, 90000, 100);
  freq_selector.setIncrement(100);

  amp_selector.init(.05, 3.5, 1);
  amp_selector.setIncrement(.1);

  mode_selector.init(lcr_param_lookup, LCR_FUNC_NUM);

}


uint8_t runSelectorTouchscreen() {
  
  if (board.ts_x > SCREEN_WIDTH/2 - SELECTOR_BUTTON_WIDTH/2 - 20 && 
      board.ts_x < SCREEN_WIDTH/2 + SELECTOR_BUTTON_WIDTH/2 + 20 &&
      board.ts_y > SELECTOR_UP_Y_POS - SELECTOR_BUTTON_HEIGHT/2 - 20 && 
      board.ts_y < SELECTOR_UP_Y_POS + SELECTOR_BUTTON_HEIGHT/2 + 20) {

    touch_to_process = false;
    return 1;
    
  } else if (board.ts_x > SCREEN_WIDTH/2 - SELECTOR_BUTTON_WIDTH/2 - 20 && 
             board.ts_x < SCREEN_WIDTH/2 + SELECTOR_BUTTON_WIDTH/2 + 20 &&
             board.ts_y > SELECTOR_DOWN_Y_POS - SELECTOR_BUTTON_HEIGHT/2 - 20 && 
             board.ts_y < SELECTOR_DOWN_Y_POS + SELECTOR_BUTTON_HEIGHT/2 + 20) {
              
    touch_to_process = false;
    return 2;            
  } 
  
  return 0;
}

void runMenuInterface() {
  uint8_t res1 = 0;
  uint8_t res2 = 0;
  uint8_t res;
  
  if (touch_to_process) {
    res1 = current_menu->processTouch(board.ts_x, board.ts_y);
    if (res1 != 0) touch_to_process = false;
  } 

  if (board.up_button.pressed() || board.up_button.process_hold()) {
    res2 = current_menu->moveUp();
  } else if (board.down_button.pressed() || board.down_button.process_hold()) {
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
  
  if (touch_to_process) {
    res1 = current_menu->processTouch(board.ts_x, board.ts_y);
    if (res1 != 0) touch_to_process = false;
  }  

  if (touch_to_process) {
    uint8_t temp = runSelectorTouchscreen();
    res2 = min(temp, 1);
    if (temp == 1) current_selector->incrementUp();
    else if (temp == 2) current_selector->incrementDown();
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
  
  if (touch_to_process) {
    res1 = current_menu->processTouch(board.ts_x, board.ts_y);
    if (res1 != 0) touch_to_process = false;
  } 

  if (touch_to_process) {
    uint8_t temp = runSelectorTouchscreen();
    res2 = min(temp, 1);
    if (temp == 1) mode_selector.incrementModeUp();
    else if (temp == 2) mode_selector.incrementModeDown();
  }

  if (board.up_button.pressed() || board.up_button.process_hold()) {
    mode_selector.incrementModeUp();
    res2 = 1;
  } else if (board.down_button.pressed() || board.down_button.process_hold()) {
    mode_selector.incrementModeDown();
    res2 = 1;
  } else if (board.enter_button.pressed()) {
    //res2 = current_menu->enter();
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

  if (exit_selector) exitModeSelector();
 
}


void runProbeToggle() {
  if (!touch_to_process) return;

  if (board.ts_x < max(0, PROBE_BMP_X_POS - 15)  || board.ts_x > PROBE_BMP_X_POS + PROBE_BMP_WIDTH + 15) return;
  if (board.ts_y < max(0, PROBE_BMP_Y_POS - 15)  || board.ts_y > PROBE_BMP_Y_POS + PROBE_BMP_HEIGHT + 15) return;

  touch_to_process = false;
  
  if (current_probes == CLIP_PROBES) current_probes = TWEEZER_PROBES;
  else current_probes = CLIP_PROBES;

  loadCalibration();
  resetAverageZ();

  if (num_cal_points == 0) {
    switchToCalMenu();
  } else {
    float freq = codecGetFrequency();
    loadCalibrationPoint(freq);
  }
  
  board.buzzer.setBuzzer(1, 10, 1);
}

void runSystem() {

  if (board.tsPressed()) touch_to_process = true;
  else touch_to_process = false;
  
  switch(current_state) {
    
    case RUNNING:
      runLCR();
      runMenuInterface();
      runProbeToggle();
      break;

    case CALIBRATION:
      runMenuInterface();
      runProbeToggle();
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

    case FREQ_SWEEP:
      runMenuInterface();
      break;
      
    default:
      break;
    
  }
  
  board.buzzer.runBuzzer(micros());
}
