#ifndef _MENUBAR_
#define _MENUBAR_

#include <Arduino.h>
#include "SPI.h"
#include "ILI9341_t3n.h"
#include <XPT2046_Touchscreen.h>


#define MAX_CATEGORIES  8
#define MAX_ITEMS       8



typedef struct menu_item_struct {
  char display_text[16];
  float *var;
  float value;
  void (*func)();
  bool is_function;
} menu_item_t;


class MenuBar {
  
  public:
    void init(uint16_t screen_width, uint16_t screen_height, uint16_t tab_height);
    void configColors(uint16_t text_color, uint16_t category_color, uint16_t item_color, uint16_t item_selected_color, uint16_t border_color);
    
    //Add caterogy. Up to 8
    void addCategory(const char *text);
    
    //Up to 8 items per category. Adds to most recently created category
    void addItem(const menu_item_t &item);

    //Either open or close the 
    void toggleCategory(uint8_t category_index);

    void moveUp();

    void moveDown();

    //Select an item from the currently open category
    void executeItem(uint8_t item_index);

    void drawMenu(ILI9341_t3n &tft);

    void processTouch(uint16_t x, uint16_t y);

    
  private:
    
    uint16_t screen_width;
    uint16_t screen_height;

    uint16_t tab_height;
    
    uint8_t num_categories;

    //currently selected category
    int8_t category_selected;

    //stores index of currently selected item
    uint8_t item_selected[MAX_CATEGORIES];

    uint8_t item_counts[MAX_CATEGORIES];
    
    char categories[MAX_CATEGORIES][16];
    menu_item_t items[MAX_CATEGORIES][MAX_ITEMS];

    uint16_t text_color;
    uint16_t category_color;
    uint16_t item_color;
    uint16_t item_selected_color;
    uint16_t border_color;
    
};


#endif
