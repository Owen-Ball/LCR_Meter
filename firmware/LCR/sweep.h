#ifndef _SWEEP_
#define _SWEEP_

#include <CComplex.h>
#include "ILI9341_t3n.h"

void setFreqSweepPoints(float points);

void setFreqSweepDisplayMode(float mode);

void runSweepWrapper(float _);

void drawSweepWrapper(ILI9341_t3n &tft);

#endif
