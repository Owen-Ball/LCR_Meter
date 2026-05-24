#include "MenuBar.h"
#include "LCR_Fonts/FreeMono9pt7b.h"

void MenuBar::init(uint16_t screen_width, uint16_t screen_height, uint16_t category_height, uint16_t item_height) {
    this->screen_width = screen_width;
    this->screen_height = screen_height;
    this->category_height = category_height;
    this->item_height = item_height;
    num_categories = 0;
    category_selected = -1;
    memset(item_counts, 0, sizeof(item_counts));
    memset(item_hovered, 0, sizeof(item_hovered));
    memset(item_selected, 0, sizeof(item_selected));

    text_color = ILI9341_WHITE;
    category_color = ILI9341_DARKGREY;
    item_color = 0x4800;
    item_selected_color = ILI9341_RED;
    border_color = text_color = ILI9341_WHITE;
}

void MenuBar::configColors(uint16_t text_color, uint16_t category_color, uint16_t item_color, uint16_t item_selected_color, uint16_t border_color) {
    this->text_color = text_color;
    this->category_color = category_color;
    this->item_color = item_color;
    this->item_selected_color = item_selected_color;
    this->border_color = border_color;
}

void MenuBar::addCategory(const char *text, void (*func)(), bool show_selection) {
    if (num_categories >= MAX_CATEGORIES) return;
    //Store the display text
    strncpy(categories[num_categories], text, 15);
    category_funcs[num_categories] = func;
    show_selections[num_categories] = show_selection;
    num_categories++;
}

uint8_t MenuBar::getCategoriesCount() {
  return num_categories;
}

void MenuBar::addItem(const menu_item_t &item) {
    //Get the category to add the item to
    uint8_t category = num_categories - 1;

    //Sanity checks
    if (category >= MAX_CATEGORIES) return;
    if (item_counts[category] >= MAX_ITEMS) return;

    //Add item to the current category
    items[category][item_counts[category]] = item;
    item_counts[category]++;
}

void MenuBar::addItem(const char *text, void (*func)(float f), float val) {
    menu_item_t item;
    strncpy(item.display_text, text, 15);
    item.value = val;
    item.func = func;
    addItem(item);
}

void MenuBar::toggleCategory(uint8_t category_index) {
  if (category_index >= num_categories) return;
  if (category_selected == category_index) {
    category_selected = -1;
  } else {
    category_selected = category_index;
    if (item_counts[category_selected] == 0) executeItem(0);
    else item_hovered[category_selected] = item_selected[category_selected];
  }
}

void MenuBar::moveUp() {
  if ((category_selected != -1) && (item_hovered[category_selected] < item_counts[category_selected] - 1)) {
    item_hovered[category_selected] += 1;
  }
}

void MenuBar::moveDown() {
  if ((category_selected != -1) && (item_hovered[category_selected] > 0)) {
    item_hovered[category_selected] -= 1;
  }
}

void MenuBar::executeItem(uint8_t cat_index, uint8_t item_index) {
  if (cat_index < num_categories) {
    category_selected = cat_index;
  }
  executeItem(item_index);
}

void MenuBar::executeItem(uint8_t item_index) {
  if (category_selected == -1) return;
  

  if (item_counts[category_selected] != 0) {
    menu_item_t item = items[category_selected][item_index];
    item_selected[category_selected] = item_index;
    memcpy(selected_text[category_selected], item.display_text, sizeof(selected_text[category_selected]));

    if (item.func != nullptr) item.func(item.value);
  } 

  
  if (category_funcs[category_selected] != nullptr) {
    category_funcs[category_selected]();
  }
  
  category_selected = -1;
   
}



void MenuBar::setSelectedText(const char *text) {
  setSelectedText(text, category_selected);
}

void MenuBar::setSelectedText(const char *text, uint8_t cat_index) {
  strncpy(selected_text[cat_index], text, 15);
}



void MenuBar::drawMenu(ILI9341_t3n &tft) {
    uint16_t tab_width = screen_width / num_categories;
    uint16_t tab_y = screen_height - category_height; //top of tab

    int16_t x1, y1;
    uint16_t w, h, text_x, text_y;

    tft.setTextColor(text_color);   // set color
    tft.setFont(&FreeMono9pt7b);
    tft.setTextSize(1);

    for (uint8_t i = 0; i < num_categories; i++) {
        uint16_t tab_x = i * tab_width;

        // Draw tab background
        tft.fillRect(tab_x, tab_y, tab_width, category_height, category_color);
        tft.drawRect(tab_x, tab_y, tab_width, category_height, border_color);

        // Draw label
        tft.getTextBounds(categories[i], 0, 0, &x1, &y1, &w, &h);
        text_x = tab_x + (tab_width - w) / 2 - 1;
        if (show_selections[i]) text_y = tab_y + 5;
        else text_y = tab_y + (category_height - h) / 2 - y1;
        tft.setCursor(text_x, text_y);
        tft.print(categories[i]);

        if (show_selections[i]) {
          tft.getTextBounds(selected_text[i], 0, 0, &x1, &y1, &w, &h);
          text_x = tab_x + (tab_width - w) / 2 - 1;
          tft.setCursor(text_x, tab_y + category_height/2 + 4);
          tft.print(selected_text[i]);
        }
    }

    if (category_selected == -1) return;

    uint16_t tab_x = category_selected * tab_width;
    for (uint8_t i = 0; i < item_counts[category_selected]; i++) {
      tab_y -= item_height;

      if (i == item_hovered[category_selected]) {
        tft.fillRect(tab_x, tab_y, tab_width, item_height, item_selected_color);
      } else {
        tft.fillRect(tab_x, tab_y, tab_width, item_height, item_color);
      }
      
      tft.drawRect(tab_x, tab_y, tab_width, item_height, border_color);
      
      tft.getTextBounds(items[category_selected][i].display_text, 0, 0, &x1, &y1, &w, &h);
      text_x = tab_x + (tab_width - w) / 2 - 1;
      text_y = tab_y + (item_height - h) / 2 - y1;
      tft.setCursor(text_x, text_y);
      tft.print(items[category_selected][i].display_text);
    }
}

void MenuBar::processTouch(uint16_t x, uint16_t y) {
  bool missed_menu = false;
  uint16_t tab_width = screen_width / num_categories;
  uint8_t category = x / tab_width;
  if (category >= num_categories) category = num_categories - 1;

  //Selected bottom menu. Toggle selected category
  if (y > screen_height - category_height) {
    toggleCategory(category);

  //Clicked above bottom menu in the selected category
  } else if (category == category_selected) {
    uint8_t item = (screen_height - y - category_height)/item_height;

    //Check if touch was above top item
    if (item_counts[category] == 0 || item > item_counts[category] - 1){
      missed_menu = true;

    //Process touch on item
    } else {

      //item touched was not currently selected
      if (item_hovered[category_selected] != item) {
        item_hovered[category_selected] = item;

      //item touched was selected. Execute it
      //touches can sometimes miss by a bit, so this ensures you need to double tap to select item
      } else {
        executeItem(item);
      }
    }
  //Missed menu entirely
  } else {
    missed_menu = true;
  }

  if (missed_menu && category_selected != -1) {
    toggleCategory(category_selected);
  }
  
}
