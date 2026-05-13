#include "board.h"
#include <MCP9800.h>


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

  setLCRRange(LCR_RANGE_100);
  setPGAGainV(PGA_GAIN_1);
  setPGAGainI(PGA_GAIN_1);

  temp_sensor.begin();
  temp_sensor.writeConfig(MCP9800::ADC_RES_12BITS);
  
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
