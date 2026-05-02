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
const int NUM_BLOCKS = 128;
float rmsAccum = 0;
int count = 0;

float avg_read2 = 0;
float rmsAccum2 = 0;
int count2 = 0;

float avgRms = 0;
float avgRms2 = 0;


// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=376,266
AudioAnalyzeRMS          rms1;            //xy=852,528
AudioAnalyzeRMS          rms2;            //xy=852,528
AudioOutputI2Sslave      i2sslave2;      //xy=562,283
AudioInputI2Sslave       i2sslave1;      //xy=619,532
AudioConnection          patchCord2(i2sslave1, 0, rms1, 0);
AudioConnection          patchCord3(i2sslave1, 1, rms2, 0);
AudioConnection          patchCord1(sine1, 0, i2sslave2, 0);
AudioControlCS4272_192k       CS4272;         //xy=580,165
// GUItool: end automatically generated code



void setup() {


  delay(500);
  
  board.init();
  
  board.buzzer.runBuzzerBlocking(9, 10, 100);

  
  AudioMemory(20);
  
  CS4272.enable();
  sine1.frequency(75000);
  sine1.amplitude(.1);
  board.increaseVGain();
  board.increaseVGain();
  //board.increaseVGain();
  board.increaseIGain();
  board.increaseIGain();
  //board.increaseIGain();
  board.setLCRRange(LCR_RANGE_1K);

  Serial.begin(115200);
  
  
}

void loop() {

  //sine1.amplitude(.1*(.5*(1+.5*cos(.002*millis()))));

  if (rms1.available()) {
    rmsAccum += rms1.read();
    count++;
    if (count >= NUM_BLOCKS) {
        avgRms = rmsAccum / float(NUM_BLOCKS);
        rmsAccum = 0;
        count = 0;
        Serial.println(String(avgRms*100,4) + " " + String(avgRms2*100,4));  
    }
  }

  if (rms2.available()) {
    rmsAccum2 += rms2.read();
    count2++;
    if (count2 >= NUM_BLOCKS) {
        avgRms2 = rmsAccum2 / float(NUM_BLOCKS);
        rmsAccum2 = 0;
        count2 = 0;
        Serial.println(String(avgRms*100,4) + " " + String(avgRms2*100,4)); 
        //Serial.println(board.getTemperature());
    }
  }
  
  // put your main code here, to run repeatedly:
  //buzzer.runBuzzer(micros());
  
  //Serial.println(millis());

  //delay(1000);
  
}
