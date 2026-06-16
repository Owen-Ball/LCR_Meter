#include "selector.h"

void Selector::init(ILI9341_t3n &tft, float min_val, float max_val, float init_val) {
    this->tft = &tft;
    this->min_val = min_val;
    this->max_val = max_val;
    this->val = init_val;
    this->increment = 1.0f;
}

void Selector::setIncrement(float increment) {
    this->increment = increment;
}

void Selector::incrementUp() {
    val += increment;
    if (val > max_val) val = max_val;
}

void Selector::incrementDown() {
    val -= increment;
    if (val < min_val) val = min_val;
}

float Selector::getValue() {
    return val;
}
