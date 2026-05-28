#include "fsm.h"
#include "constants.h"
#include "calibration.h"
#include "board.h"
#include "codec.h"
#include "lcr_func.h"


MenuBar calibration_menu;
MenuBar main_menu_1;
MenuBar main_menu_2;

MenuBar *current_menu;

SYSTEM_STATE current_state;


void switchMainMenuPage() {
  if (current_menu == &main_menu_1) current_menu = &main_menu_2;
  else current_menu = &main_menu_1;
}

void switchToCalMenu() {
  current_menu = &calibration_menu;
}

void switchToMainMenu() {
  //Do not go to main LCR page if cal data is not loaded
  if (num_cal_points == 0) return;
  
  current_menu = &main_menu_1;
}

void calibrateProbesQuick() {
  
}

void initCalMenu() {
  calibration_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);
  
  calibration_menu.addCategory("Probe", nullptr, false, false);
  calibration_menu.addItem("Quick", &calibrateProbes_Point, 0.0f);
  calibration_menu.addItem("Full", &calibrateProbes, 0.0f);

  calibration_menu.addCategory("All", nullptr, false, false);
  calibration_menu.addItem("Quick", nullptr, 0.0f);
  calibration_menu.addItem("Full", nullptr, 0.0f);
  
  calibration_menu.addCategory("Save", nullptr, false);
  calibration_menu.addItem("Confirm", nullptr, 0.0f);

  calibration_menu.addCategory("Home", &switchToMainMenu, false);

}

void initMainMenu1() {
  main_menu_1.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);

  main_menu_1.addCategory("Freq");
  main_menu_1.addItem("100Hz", &setLCRFrequency, 100.0f);
  main_menu_1.addItem("1kHz", &setLCRFrequency, 1000.0f);
  main_menu_1.addItem("10kHz", &setLCRFrequency, 10000.0f);
  main_menu_1.addItem("75kHz", &setLCRFrequency, 75000.0f);
  main_menu_1.addItem("Custom", nullptr);
  main_menu_1.executeItem(main_menu_1.getCategoriesCount()-1, 1);

  main_menu_1.addCategory("Mode");
  main_menu_1.addItem("Auto", nullptr, 0.0f);
  main_menu_1.addItem("Cs+Rs", nullptr, 0.0f);
  main_menu_1.addItem("Ls+Rs", nullptr, 0.0f);
  main_menu_1.addItem("Cp+Rp", nullptr, 0.0f);
  main_menu_1.addItem("Lp+Rp", nullptr, 0.0f);
  main_menu_1.executeItem(main_menu_1.getCategoriesCount()-1, 0);

  main_menu_1.addCategory("Cal", &switchToCalMenu, false);

  main_menu_1.addCategory("Page 2", &switchMainMenuPage, false);
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
  main_menu_2.executeItem(main_menu_2.getCategoriesCount()-1, 3);

  main_menu_2.addCategory("Other");
  main_menu_2.addItem("0.1V", nullptr, 0.1f);
  main_menu_2.addItem("0.5V", nullptr, 0.5f);
  main_menu_2.addItem("1.0V", nullptr, 1.0f);
  main_menu_2.addItem("2.0V", nullptr, 2.0f);
  main_menu_2.executeItem(main_menu_2.getCategoriesCount()-1, 3);

  main_menu_2.addCategory("Page 1", &switchMainMenuPage, false);
}


void initSystem() {

  initCalMenu();
  initMainMenu1();
  initMainMenu2();

  current_menu = &main_menu_1;

  board.tft.fillScreen(ILI9341_BLACK);
  board.tft.updateScreenAsync();

  if (points_loaded == 0) {
    current_state = RUNNING;
  } else {
    current_state = CALIBRATION;
  }
  
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


void runSystem() {

  switch(current_state) {
    
    case RUNNING:
      runMenuInterface();
      break;

    case CALIBRATION:
      runMenuInterface();
      break;
      
    default:
      break;
    
  }
  
  board.buzzer.runBuzzer(micros());
}
