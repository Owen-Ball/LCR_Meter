#include "buzzer.h"
#include "board.h"
#include "codec.h"
#include "calibration.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


void setup() {


  delay(500);
  
  board.init();
  

  codecInit();
  codecSetOutputFrequency(100);
  codecSetOutputAmplitude(.8);
  
  //board.increaseVGain();
  //board.increaseVGain();
  //board.increaseVGain();
  //board.increaseIGain();
  //board.increaseIGain();
  //board.increaseIGain();
  board.setLCRRange(LCR_RANGE_100);

  loadCalibrationPoint();

  Serial.begin(115200);

  delay(500);
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  calibrateAll();
  
}

void loop() {

  //codecAverageReadings();
  codecBlockingMeasure();
  //Serial.println(board.getTemperature());
  Serial.println(String(100*codecReadings.i_rms, 5));

  //calibrateIPGA_Full(calibration_data);
  
}
