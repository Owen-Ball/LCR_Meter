#ifndef _FSM_
#define _FSM_

//Store settings to reload after coming out of calibration menu
typedef struct system_settings_struct {
  float freq;
  float amp;
} system_settings_t;


void initSystem();
void runSystem();


#endif
