#include "buzzer.h"
#include "board.h"
#include "codec.h"
#include "calibration.h"
#include "autorange.h"
#include "lcr_func.h"

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
  board.setLCRRange(LCR_RANGE_10K);


  Serial.begin(115200);

  delay(500);
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  //calibrateAll();
  //saveCalibration();
  loadCalibration();
  loadCalibrationPoint(1000);
  printCalibrationPoint(calibration_data);
  //calibrateProbeQuick();

  board.setPGAGainV(PGA_GAIN_1);
  board.setPGAGainI(PGA_GAIN_1);
  
  delay(2000);
  board.setLCRRange(LCR_RANGE_10K);
  codecSetOutputFrequency(1000);
  codecSetOutputAmplitude(.8);
  
}

void loop() {

  //calibrateIPGA();
  //blockingAutorangeMeasure();
  codecAverageReadings();
  if (codecDataAvailable) {
    //Serial.print(codecReadings.i_rms);
    //Serial.print(" ");
    //Serial.print(codecReadings.v_rms);
    //Serial.print(" ");
    Serial.print(board.getPGAGainI());
    Serial.print(" ");
    Serial.print(board.getPGAGainV());
    Serial.print(" ");
    //Serial.print(codecReadings.v_rms / codecReadings.i_rms * 100000.0);
    //Serial.print(" ");
    //Serial.print(codecReadings.phase * 57.3);
    Serial.print(board.getLCRRange());
    //Serial.print(RANGE_RESISTOR[board.getLCRRange()]);
    Serial.print(" ");
    Serial.print(calculateZ());
    Serial.println(" ");
    //Serial.println(getCs(calculateZ(), 75000.0) * 1e12);
    bool gain_ranged = gainAutorange(false);
    if (!gain_ranged) rangeAutorange(false);
    rangeAutorange(false);
    codecResetReadings();
  }

  //codecBlockingMeasure();
  //Serial.println(board.getTemperature());
  //Serial.println(String(100*codecReadings.i_rms, 5));

  //calibrateIPGA_Full(calibration_data);
  
}
