#include "buzzer.h"
#include "board.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "audio/control_cs4272_192k.h"
#include "audio/control_cs4272_192k.cpp"


float avg_read = 0;
const int NUM_BLOCKS = 64;
float rmsAccum = 0;
int count = 0;


// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=376,266
AudioAnalyzeRMS          rms1;            //xy=852,528
AudioOutputI2Sslave      i2sslave2;      //xy=562,283
AudioInputI2Sslave       i2sslave1;      //xy=619,532
AudioConnection          patchCord2(i2sslave1, 0, rms1, 0);
AudioConnection          patchCord1(sine1, 0, i2sslave2, 0);
AudioControlCS4272_192k       CS4272;         //xy=580,165
// GUItool: end automatically generated code



void setup() {
  
  delay(500);
  board.init();
  AudioMemory(20);
  
  CS4272.enable();
  sine1.frequency(50000);
  sine1.amplitude(1);
  Serial.begin(115200);
  
  
}

void loop() {

  sine1.amplitude(.5*(1+.5*cos(.005*millis())));

  if (rms1.available()) {
    rmsAccum += rms1.read();
    count++;
    if (count >= NUM_BLOCKS) {
        float avgRms = rmsAccum / float(NUM_BLOCKS);
        rmsAccum = 0;
        count = 0;
        Serial.println(avgRms*100,4); 
    }
  }
  
  // put your main code here, to run repeatedly:
  //buzzer.runBuzzer(micros());
  
  //board.buzzer.runBuzzerBlocking(1, 7, 200);
  
  //Serial.println(millis());

  //delay(1000);
  
}
