#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "control_cs4272_192k.h"

const int chipSelect = BUILTIN_SDCARD;

float avg_read = 0;
const int NUM_BLOCKS = 64;
float rmsAccum = 0;
int count = 0;

String dataString = "";

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
  AudioMemory(20);
  CS4272.enable();
  sine1.frequency(50000);
  sine1.amplitude(1);
  Serial.begin(115200);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
    }
  }
  Serial.println("card initialized.");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //sine1.frequency(50000*(1+.8*cos(.005*millis())));
  sine1.amplitude(.5*(1+.5*cos(.005*millis())));
  if (rms1.available()) {
    rmsAccum += rms1.read();
    count++;
    if (count >= NUM_BLOCKS) {
        float avgRms = rmsAccum / float(NUM_BLOCKS);
        rmsAccum = 0;
        count = 0;
        Serial.println(avgRms*100,4);
        // use avgRms
        dataString = String(avgRms*100);
        File dataFile = SD.open("datalog.txt", FILE_WRITE);

        // if the file is available, write to it:
        if (dataFile) {
          dataFile.println(dataString);
          dataFile.close();
          // print to the serial port too:
          //Serial.println(dataString);
        } else {
          // if the file isn't open, pop up an error:
          Serial.println("error opening datalog.txt");
        }
    }
  }
}
