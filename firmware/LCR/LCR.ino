#include "buzzer.h"
#include "board.h"
#include "codec.h"
#include "calibration.h"
#include "autorange.h"
#include "lcr_func.h"
#include "display.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>



FloatDisplay disp;
void setup() {


  delay(500);
  
  board.init();
  
  disp.init(0, 0, true);
  disp.configSettings(6, -12, 1e6, "F");
  
  codecInit();
  codecSetOutputFrequency(10000);
  codecSetOutputAmplitude(.8);

  board.setLCRRange(LCR_RANGE_10K);


  Serial.begin(115200);

  disp.updateValue(.000312);
  disp.updateValue(.000912);
  disp.updateValue(.001050);
  disp.updateValue(.001150);
  disp.updateValue(.85);
  disp.updateValue(.95);
  disp.updateValue(1.05);
  disp.updateValue(1.15);
  disp.updateValue(2);
  disp.updateValue(1.15);
  disp.updateValue(1.05);
  disp.updateValue(.95);
  disp.updateValue(.85);

  //delay(1000000);
  delay(500);
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  //calibrateAll();
  //saveCalibration();
  loadCalibration();
  loadCalibrationPoint(5000);
  printCalibrationPoint(calibration_data);
  calibrateProbeQuick();

  board.setPGAGainV(PGA_GAIN_1);
  board.setPGAGainI(PGA_GAIN_1);
  
  delay(2000);
  board.setLCRRange(LCR_RANGE_10K);
  codecSetOutputFrequency(5000);
  codecSetOutputAmplitude(.8);
  
}

void loop() {

  //calibrateIPGA();
  //blockingAutorangeMeasure();
  codecAverageReadings();
  if (codecDataAvailable) {
    
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

  //codecBlockingMeasure();
  //Serial.println(board.getTemperature());
  
}
