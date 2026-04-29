#include "buzzer.h"

Buzzer::Buzzer(uint8_t pin){
  this->pin = pin;
  active = false;
  high_time = 0;
  low_time = 0;
  pulse = 0;
  total_pulses = 0; 
}


void Buzzer::setBuzzer(uint8_t total_pulses, long high_time, long low_time){
  if (active) return;
  total_pulses = total_pulses;
  active = true;
  high_time = high_time*1000;
  low_time = low_time*1000;
  pulse = 0;
  start_time = micros();
  buzzer_state = false;
}


void Buzzer::runBuzzer(long t){
  bool tempvar = (t - start_time) % (low_time + high_time) < high_time;

  //only run the following code if the buzzer state has changed
  if (tempvar == buzzer_state  || !this->active) return;

  buzzer_state = tempvar;
  if (tempvar) {
    digitalWriteFast(pin, HIGH);    
    if (pulse != 255) pulse++;

  } else {
    //disable the buzzer if the number of pulses set has been reached
    if (pulse == total_pulses && total_pulses != 0) {
      active = false;
    }
    digitalWriteFast(pin, LOW);
  }
}


void Buzzer::stopBuzzer(){
  digitalWriteFast(pin, LOW);
  active = false;
  buzzer_state = false;
}


void Buzzer::runBuzzerBlocking(uint8_t total_pulses, long high_time, long low_time){
  //force number of pulses to at least 1, since otherwise this would never return
  setBuzzer(max(1,total_pulses), high_time, low_time);

  //Run buzzer until state is off
  while (active){
    long t = micros();
    runBuzzer(t);
  }
}
