#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "buzzer.h"
#include "board.h"
#include "codec.h"
#include "calibration.h"
#include "autorange.h"
#include "lcr_func.h"
#include "display.h"
#include "fsm.h"




FloatDisplay disp;
long unsigned int loop_time = 0;
long unsigned int prev_time = 0;

void setup() {


  delay(500);

  Serial.begin(115200);
  
  board.init();
  
  disp.init(0, 0, true);
  disp.configSettings(6, -12, 1e6, "F");
  
  codecInit();
  codecSetOutputFrequency(10000);
  codecSetOutputAmplitude(.8);
  board.setLCRRange(LCR_RANGE_10K);

  initSystem();

  delay(500);
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  //calibrateAll();
  //saveCalibration();
  loadCalibration();
  loadCalibrationPoint(5000);
  printCalibrationPoint(calibration_data);
 // calibrateProbeQuick();

  board.setPGAGainV(PGA_GAIN_1);
  board.setPGAGainI(PGA_GAIN_1);
  
  delay(100);
  board.setLCRRange(LCR_RANGE_10K);
  codecSetOutputFrequency(5000);
  codecSetOutputAmplitude(.8);
  
}

void loop() {

  //calibrateIPGA();
  //blockingAutorangeMeasure();
  codecAverageReadings();
  if (codecDataAvailable && 0) {
    
    Serial.print(board.getPGAGainI());
    Serial.print(" ");
    Serial.print(board.getPGAGainV());
    Serial.println(" ");
    //Serial.print(board.getLCRRange());
    //Serial.print(RANGE_RESISTOR[board.getLCRRange()]);
    //Serial.print(" ");
    //Serial.print(calculateZ());
    //Serial.println(" ");
    //Serial.println(getCs(calculateZ(), 75000.0) * 1e12);
    Serial.println(getRs(calculateZ(), 100.0));
    disp.updateValue(getCs(calculateZ(), 5000.0));
    bool gain_ranged = gainAutorange(false);
    if (!gain_ranged) rangeAutorange(false);
    rangeAutorange(false);
    codecResetReadings();
  }

  runSystem();
  
  loop_time = micros() - prev_time;
  prev_time = micros();
  //Serial.println(loop_time);

  //codecBlockingMeasure();
  //Serial.println(board.getTemperature());
  
}
