
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

  board.buzzer.runBuzzerBlocking(4, 10, 50);

  //calibrateAll();
  //saveCalibration();

  digitalWrite(FAN_PIN, HIGH);
  
  loadCalibration();
  
  initSystem();
  //calibrateProbes();
  //calibrateProbes_Point();

  
  delay(100); 
}

void loop() {

  //calibrateIPGA();
  //blockingAutorangeMeasure();
  codecAverageReadings();
  if (codecDataAvailable) {
    
    //Serial.print(board.getPGAGainI());
    //Serial.print(" ");
    //Serial.print(board.getPGAGainV());
    //Serial.println(" ");
    //Serial.print(board.getLCRRange());
    //Serial.print(RANGE_RESISTOR[board.getLCRRange()]);
    //Serial.print(" ");
    //Serial.print(calculateZ());
    //Serial.println(" ");
    //Serial.println(getCs(calculateZ(), 75000.0) * 1e12);
    Serial.println(getRs(calculateZ(), 100.0));
    Serial.println(board.getTemperature());
    disp.updateValue(getCs(calculateZ(), getLCRFrequency()));
    bool gain_ranged = gainAutorange(false);
    if (!gain_ranged) rangeAutorange(false);
    rangeAutorange(false);
    codecResetReadings();
  }

  bool update_finished = !board.tft.asyncUpdateActive();
  runSystem();
  if (update_finished) {
    drawAll();
  }

 
  loop_time = micros() - prev_time;
  prev_time = micros();
  delay(1);
  //Serial.println(loop_time);

  //codecBlockingMeasure();
  //Serial.println(board.getTemperature());
  
}
