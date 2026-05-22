#include "MenuBar.h"

void MenuBar::init(uint16_t screen_width, uint16_t screen_height, uint16_t tab_height) {
    this->screen_width = screen_width;
    this->screen_height = screen_height;
    this->tab_height = tab_height;
    num_categories = 0;
    category_selected = -1;
    memset(item_counts, 0, sizeof(item_counts));
    memset(item_selected, 0, sizeof(item_selected));

    text_color = ILI9341_WHITE;
    category_color = ILI9341_DARKGREY;
    item_color = 0x4000;
    item_selected_color = ILI9341_RED; //0xC618;
    border_color = text_color = ILI9341_WHITE;
}

void MenuBar::configColors(uint16_t text_color, uint16_t category_color, uint16_t item_color, uint16_t item_selected_color, uint16_t border_color) {
    this->text_color = text_color;
    this->category_color = category_color;
    this->item_color = item_color;
    this->item_selected_color = item_selected_color;
    this->border_color = border_color;
}

void MenuBar::addCategory(const char *text) {
    if (num_categories >= MAX_CATEGORIES) return;
    //Store the display text
    strncpy(categories[num_categories], text, 16);
    num_categories++;
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

void MenuBar::toggleCategory(uint8_t category_index) {
  if (category_index >= num_categories) return;
  if (category_selected == category_index) category_selected = -1;
  else category_selected = category_index;
}

void MenuBar::moveUp() {
  if ((category_selected != -1) && (item_selected[category_selected] < item_counts[category_selected] - 1)) {
    item_selected[category_selected] += 1;
  }
}

void MenuBar::moveDown() {
  if ((category_selected != -1) && (item_selected[category_selected] > 0)) {
    item_selected[category_selected] -= 1;
  }
}

void MenuBar::executeItem(uint8_t item_index) {
  if (category_selected == -1) return;
  
  menu_item_t item = items[category_selected][item_index];
  
  if (item.is_function) {
    item.func();
  } else {
    *(item.var) = item.value;
  }

  toggleCategory(category_selected);
}

void MenuBar::drawMenu(ILI9341_t3n &tft) {
    uint16_t tab_width = screen_width / num_categories;
    uint16_t tab_y = screen_height - tab_height; //top of tab

    for (uint8_t i = 0; i < num_categories; i++) {
        uint16_t tab_x = i * tab_width;

        // Draw tab background
        tft.fillRect(tab_x, tab_y, tab_width, tab_height, category_color);
        tft.drawRect(tab_x, tab_y, tab_width, tab_height, border_color);

        // Draw label
        tft.setTextColor(text_color);
        tft.setCursor(tab_x + 4, tab_y + 4);
        tft.print(categories[i]);
    }

    if (category_selected == -1) return;

    uint16_t item_x = category_selected * tab_width;
    for (uint8_t i = 0; i < item_counts[category_selected]; i++) {
      tab_y -= tab_height;
      if (i == item_selected[category_selected]) {
        tft.fillRect(item_x, tab_y, tab_width, tab_height, item_selected_color);
      } else {
        tft.fillRect(item_x, tab_y, tab_width, tab_height, item_color);
      }
      
      tft.drawRect(item_x, tab_y, tab_width, tab_height, border_color);
      tft.setTextColor(text_color);
      tft.setCursor(item_x + 4, tab_y + 4);
      tft.print(items[category_selected][i].display_text);
    }
}

void MenuBar::processTouch(uint16_t x, uint16_t y) {
  bool missed_menu = false;
  uint16_t tab_width = screen_width / num_categories;
  uint8_t category = x / tab_width;

  //Selected bottom menu. Toggle selected category
  if (y > screen_height - tab_height) {
    toggleCategory(category);

  //Clicked above bottom menu in the selected category
  } else if (category == category_selected) {
    uint8_t item = (screen_height - y - tab_height)/tab_height;

    //Check if touch was above top item
    if (item < 0 || item > item_counts[category] - 1){
      missed_menu = true;

    //Process touch on item
    } else {

      //item touched was not currently selected
      if (item_selected[category_selected] != item) {
        item_selected[category_selected] = item;

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
