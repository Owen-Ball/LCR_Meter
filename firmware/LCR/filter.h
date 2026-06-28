#ifndef _FILTER_
#define _FILTER_

#include <CircularBuffer.hpp>

#define MOVING_AVERAGE_MAX 64

class MovingAverage {
  public:
    MovingAverage(uint8_t width = 16);
    void add(float value);
    float getAverage();
    void setWidth(uint8_t width);
    void reset();

  private:
    CircularBuffer<float, MOVING_AVERAGE_MAX> buffer;
    uint8_t width;
    float sum;
};

#endif
