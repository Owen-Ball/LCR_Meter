#include "Buzzer.h"

Buzzer::Buzzer(uint8_t pin){
  this->pin = pin;
  this->active = false;
  this->high_time = 0;
  this->low_time = 0;
  this->pulse = 0;
  this->total_pulses = 0; 
}


void Buzzer::setBuzzer(uint8_t total_pulses, long high_time, long low_time){
  if (this->active) return;
  this->total_pulses = total_pulses;
  this->active = true;
  this->high_time = high_time*1000;
  this->low_time = low_time*1000;
  this->pulse = 0;
  this->start_time = micros();
  this->buzzer_state = false;
}


void Buzzer::runBuzzer(long t){
  bool tempvar = (t - this->start_time) % (this->low_time + this->high_time) < this->high_time;

  //only run the following code if the buzzer state has changed
  if (tempvar == this->buzzer_state  || !this->active) return;

  this->buzzer_state = tempvar;
  if (tempvar) {
    digitalWriteFast(this->pin, HIGH);

    //disable the buzzer if the number of pulses set has been reached
    if (this->pulse == this->total_pulses && this->total_pulses != 0) {
      this->active = false;
      digitalWriteFast(this->pin, LOW);
    }
    if (this->pulse != 255) this->pulse++;

  } else {
    digitalWriteFast(this->pin, LOW);
  }
}


void Buzzer::stopBuzzer(){
  digitalWriteFast(this->pin, LOW);
  this->active = false;
  this->buzzer_state = false;
}


void Buzzer::runBuzzerBlocking(uint8_t total_pulses, long high_time, long low_time){
  //force number of pulses to at least 1, since otherwise this would never return
  this->setBuzzer(max(1,total_pulses), high_time, low_time);

  //Run buzzer until state is off
  while (this->active){
    long t = micros();
    this->runBuzzer(t);
  }
}
