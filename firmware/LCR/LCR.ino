#include "Buzzer.h"


const uint8_t BUZ_PIN = 1;
Buzzer buzzer(BUZ_PIN);


void setup() {
  // put your setup code here, to run once:
  buzzer.setBuzzer(1, 400, 150);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  buzzer.runBuzzerBlocking(1, 100, 1000);
}
