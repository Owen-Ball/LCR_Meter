#include "fsm.h"
#include "menubar.h"
#include "constants.h"
#include "calibration.h"
#include "board.h"
#include "codec.h"
#include "lcr_func.h"


MenuBar calibration_menu;
MenuBar main_menu_1;
MenuBar main_menu_2;

MenuBar *current_menu;

long unsigned int prev_refresh_time = 0;

system_settings_t stored_settings;


void storeSettings() {
  stored_settings.freq = getLCRFrequency();
  stored_settings.amp = getLCRAmplitude();
}

void loadSettings() {
  setLCRFrequency(stored_settings.freq);
  setLCRAmplitude(stored_settings.amp);
}

void switchMainMenuPage() {
  if (current_menu == &main_menu_1) current_menu = &main_menu_2;
  else current_menu = &main_menu_1;
}

void switchToCalMenu() {
  current_menu = &calibration_menu;
  storeSettings();
}

void switchToMainMenu() {
  current_menu = &main_menu_1;
  loadSettings();
}

void initCalMenu() {
  calibration_menu.init(SCREEN_WIDTH, SCREEN_HEIGHT, MENU_CATEGORY_HEIGHT, MENU_ITEM_HEIGHT);
  
  calibration_menu.addCategory("Probe", nullptr, false);
  calibration_menu.addItem("Quick", nullptr, 0.0f);
  calibration_menu.addItem("Full", nullptr, 0.0f);

  calibration_menu.addCategory("All", nullptr, false);
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
  main_menu_1.executeItem(main_menu_1.getCategoriesCount()-1, 1);

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
}


void runMenuInterface() {
  if (board.tsPressed()) {
    current_menu->processTouch(board.ts_x, board.ts_y);
  }
}

void drawAll(bool force_update = false) {
  
  if (millis() - prev_refresh_time < DISP_REFRESH_TIME && !force_update) return;

  board.tft.waitUpdateAsyncComplete();
  
  board.tft.fillScreen(ILI9341_BLACK);
  current_menu->drawMenu(board.tft);
  
  board.tft.updateScreenAsync();

  Serial.println(millis() - prev_refresh_time);

  prev_refresh_time = millis();
}

void runSystem() {

  bool update_finished = !board.tft.asyncUpdateActive();
  runMenuInterface();
  if (update_finished) {
    drawAll();
  }
}
