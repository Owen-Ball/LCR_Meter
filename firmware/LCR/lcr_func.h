#ifndef _LCR_
#define _LCR_
#include <Arduino.h>
#include <CComplex.h>

typedef struct lcr_param_struct {
  const char *label;
  const char *unit;
  int8_t resolution;
  float (*value)(Complex z, float f);
} lcr_param_t;

extern lcr_param_t primary_lcr_param;
extern lcr_param_t secondary_lcr_param;

extern float primary_lcr_value;
extern float secondary_lcr_value;


void setLCRFrequency(float freq);
void setLCRAmplitude(float amp);
float getLCRFrequency();
float getLCRAmplitude();

void runLCR();

Complex calculateZ();

float getPhasorAngle(Complex Z, float freq);

float getPhasorMag(Complex Z, float freq);

float getRs(Complex Z, float freq);

float getCs(Complex Z, float freq);

float getLs(Complex Z, float freq);

float getRp(Complex Z, float freq);

float getCp(Complex Z, float freq);

float getLp(Complex Z, float freq);

extern lcr_param_t lcrParamCs;
extern lcr_param_t lcrParamCp;
extern lcr_param_t lcrParamLs;
extern lcr_param_t lcrParamLp;
extern lcr_param_t lcrParamRs;
extern lcr_param_t lcrParamRp;
extern lcr_param_t lcrParamZMag;
extern lcr_param_t lcrParamZPhase;
extern lcr_param_t lcrParamD;
extern lcr_param_t lcrParamQ;

#endif
