#ifndef _FSM_
#define _FSM_

#include "menubar.h"

enum SYSTEM_STATE {RUNNING, CALIBRATION, USER_INPUT};

extern SYSTEM_STATE current_state;
extern MenuBar *current_menu;

void initSystem();
void runSystem();


#endif
