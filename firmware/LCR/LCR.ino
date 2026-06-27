
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


long unsigned int loop_time = 0;
long unsigned int prev_time = 0;

void setup() {

  delay(500);

  Serial.begin(115200);
  
  board.init();
  codecInit();

  initDraw();
  initSystem();

  board.buzzer.runBuzzerBlocking(4, 10, 50);

  digitalWrite(FAN_PIN, HIGH);

  delay(100); 
}

void loop() {

  bool update_finished = !board.tft.asyncUpdateActive();
  
  runSystem();
  if (update_finished) {
    drawAll();
  }

  loop_time = micros() - prev_time;
  prev_time = micros();
  Serial.println(loop_time);
}
