#include "Buzzer.h"


const uint8_t BUZ_PIN = 33;
Buzzer buzzer(BUZ_PIN);


void setup() {
  // put your setup code here, to run once:
  //buzzer.setBuzzer(BUZ_PIN, 400, 150);
  Serial.begin(115200);
  pinMode(BUZ_PIN, OUTPUT);
  //buzzer.setBuzzer(0, 100, 300);
}

void loop() {
  // put your main code here, to run repeatedly:
  //buzzer.runBuzzer(micros());
  buzzer.runBuzzerBlocking(1, 7, 200);
  //Serial.println(millis());

  delay(1000);
  
}
