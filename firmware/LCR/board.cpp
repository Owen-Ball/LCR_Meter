#include "board.h"


DMAMEM uint16_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

Board board;

void Board::init() {
  pinMode(RANGE_SEL_0_PIN, OUTPUT);
  pinMode(RANGE_SEL_1_PIN, OUTPUT);
  pinMode(PGA_V_0_PIN, OUTPUT);
  pinMode(PGA_V_1_PIN, OUTPUT);
  pinMode(PGA_I_0_PIN, OUTPUT);
  pinMode(PGA_I_1_PIN, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  enter_button.begin();
  up_button.begin();
  down_button.begin();
  select_button_1.begin();
  select_button_2.begin();
  select_button_3.begin();
  select_button_4.begin();

  setLCRRange(LCR_RANGE_100);
  setPGAGainV(PGA_GAIN_1);
  setPGAGainI(PGA_GAIN_1);

  temp_sensor.begin();
  temp_sensor.writeConfig(MCP9800::ADC_RES_12BITS);

  tft.begin(16000000);
  tft.setRotation(3);
  tft.setFrameBuffer(framebuffer);
  tft.useFrameBuffer(true);
  tft.updateChangedAreasOnly(true);

  ts.begin();
  ts.setRotation(1);

  ts_ignoreuntil = 0;
  ts_state = false;
  
}


void Board::setLCRRange(uint8_t range) {
  board_range = range & 0x03;
  digitalWrite(RANGE_SEL_0_PIN, (board_range >> 0) & 0x01);
  digitalWrite(RANGE_SEL_1_PIN, (board_range >> 1) & 0x01);
}

void Board::setPGAGainV(uint8_t gain) {
  board_gain_v = gain & 0x03;
  digitalWrite(PGA_V_0_PIN, (board_gain_v >> 0) & 0x01);
  digitalWrite(PGA_V_1_PIN, (board_gain_v >> 1) & 0x01);
}

void Board::setPGAGainI(uint8_t gain) {
  board_gain_i = gain & 0x03;
  digitalWrite(PGA_I_0_PIN, (board_gain_i >> 0) & 0x01);
  digitalWrite(PGA_I_1_PIN, (board_gain_i >> 1) & 0x01);
}

bool Board::increaseVGain() {
  if (board_gain_v < PGA_GAIN_NUM-1) {
    setPGAGainV(board_gain_v + 1);
    return true;
  }
  return false;
}

bool Board::decreaseVGain() {
  if (board_gain_v > 0) {
    setPGAGainV(board_gain_v - 1);
    return true;
  }
  return false;
}

bool Board::increaseIGain() {
  if (board_gain_i < PGA_GAIN_NUM-1) {
    setPGAGainI(board_gain_i + 1);
    return true;
  }
  return false;
}

bool Board::decreaseIGain() {
  if (board_gain_i > 0) {
    setPGAGainI(board_gain_i - 1);
    return true;
  }
  return false;
}

bool Board::increaseLCRRange() {
  if (board_range < LCR_RANGE_NUM-1) {
    setLCRRange(board_range + 1);
    return true;
  }
  return false;
}

bool Board::decreaseLCRRange() {
  if (board_range > 0) {
    setLCRRange(board_range - 1);
    return true;
  }
  return false;
}

float Board::getTemperature() {
  return temp_sensor.readTempC16(MCP9800::AMBIENT) / 16.0;
}

uint16_t boundedMap(uint16_t x, uint16_t a1, uint16_t a2, uint16_t b1, uint16_t b2) {
  if (x < a1) return b1;
  if (x > a2) return b2;
  return map(x, a1, a2, b1, b2); 
}

bool Board::tsPressed(long unsigned int debounce_ms) {
  if (tft.asyncUpdateActive()) return false;
  if (millis() < ts_ignoreuntil) return false;

  bool curr_state = ts.touched();

  if (curr_state == ts_state) return false;
  
  ts_ignoreuntil = millis() + debounce_ms;
  ts_state = curr_state;
  
  if (curr_state) {
    TS_Point p = ts.getPoint();
    ts_x = boundedMap(p.x, TS_X_MIN, TS_X_MAX, 0, SCREEN_WIDTH);
    ts_y = boundedMap(p.y, TS_Y_MIN, TS_Y_MAX, 0, SCREEN_HEIGHT);
    return true;
  } else {
    return false;
  }

}
