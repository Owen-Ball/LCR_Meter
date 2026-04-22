#ifndef _BUZZER_
#define _BUZZER_
#include <Arduino.h>

class Buzzer {
  public:
    Buzzer(uint8_t pin);
    void setBuzzer(uint8_t total_pulses, long high_time, long low_time); //0-255 for # of pulses, 0 is on forever. Times in ms
    void stopBuzzer();
    void runBuzzer(long t); //time from micros() in main loop
    void runBuzzerBlocking(uint8_t total_pulses, long high_time, long low_time); //0-255 for # of pulses, 0 is on forever. Times in ms

    bool active; 
                 
  private:
    uint8_t pin;
    uint8_t pulse;
    uint8_t total_pulses;
    bool buzzer_state;
    long high_time;
    long low_time;
    long start_time;    
            
};

#endif
