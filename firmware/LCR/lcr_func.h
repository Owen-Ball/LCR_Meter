#ifndef _LCR_
#define _LCR_
#include <Arduino.h>
#include <CComplex.h>

void setLCRFrequency(float f);

Complex calculateZ();

float getPhasorAngle(Complex Z, float freq);

float getPhasorMag(Complex Z, float freq);

float getRs(Complex Z, float freq);

float getCs(Complex Z, float freq);

float getLs(Complex Z, float freq);

float getRp(Complex Z, float freq);

float getCp(Complex Z, float freq);

float getLp(Complex Z, float freq);

#endif
