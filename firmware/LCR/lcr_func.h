#ifndef _LCR_
#define _LCR_
#include <Arduino.h>
#include <CComplex.h>
#include "constants.h"

typedef struct lcr_param_struct {
  const char *label;
  const char *unit;
  int8_t min_exp;
  int8_t min_res;
  float (*value)(Complex z, float f);
} lcr_param_t;

extern lcr_param_t primary_lcr_param;
extern lcr_param_t secondary_lcr_param;

extern bool auto_param;

extern lcr_param_t *lcr_param_lookup[LCR_FUNC_NUM];

extern float primary_lcr_value;
extern float secondary_lcr_value;
extern float resistance_lcr_value;
extern float reactance_lcr_value;


void setLCRFrequency(float freq);
void setLCRAmplitude(float amp);
float getLCRFrequency();
float getLCRAmplitude();

void setAverageZ(float count);
void resetAverageZ();

void runLCR();
void setLCRParams(int index);
void setLCRParams(float f);

Complex calculateZ();

float getQ(Complex Z, float freq);

float getD(Complex Z, float freq);

float getPhasorAngle(Complex Z, float freq);

float getPhasorMag(Complex Z, float freq);

float getRs(Complex Z, float freq);

float getCs(Complex Z, float freq);

float getLs(Complex Z, float freq);

float getRp(Complex Z, float freq);

float getCp(Complex Z, float freq);

float getLp(Complex Z, float freq);

float getXs(Complex Z, float freq);

float getRs_signed(Complex Z, float freq);

extern lcr_param_t lcrParamCs;
extern lcr_param_t lcrParamCp;
extern lcr_param_t lcrParamLs;
extern lcr_param_t lcrParamLp;
extern lcr_param_t lcrParamRs;
extern lcr_param_t lcrParamRp;
extern lcr_param_t lcrParamXs;
extern lcr_param_t lcrParamZMag;
extern lcr_param_t lcrParamZPhase;
extern lcr_param_t lcrParamD;
extern lcr_param_t lcrParamQ;

#endif
