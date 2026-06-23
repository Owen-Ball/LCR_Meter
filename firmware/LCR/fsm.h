#ifndef _FSM_
#define _FSM_

#include "menubar.h"
#include "selector.h"

enum SYSTEM_STATE {RUNNING, CALIBRATION, FREQ_INPUT, AMP_INPUT, MODE_INPUT};

extern SYSTEM_STATE current_state;
extern MenuBar *current_menu;

extern Selector freq_selector;
extern Selector amp_selector;
extern Selector *current_selector;

void initSystem();
void runSystem();


#endif
