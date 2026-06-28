#include "filter.h"


MovingAverage::MovingAverage(uint8_t width) {
    this->width = width;
    this->sum = 0.0f;
}

void MovingAverage::add(float value) {
    if (buffer.size() >= width) {
        sum -= buffer.shift();
    }
    buffer.push(value);
    sum += value;
}

float MovingAverage::getAverage() {
    if (buffer.isEmpty()) return 0.0f;
    return sum / buffer.size();
}

void MovingAverage::setWidth(uint8_t width) {
    this->width = width;
    reset();
}

void MovingAverage::reset() {
    buffer.clear();
    sum = 0.0f;
}
