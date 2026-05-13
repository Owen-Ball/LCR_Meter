#ifndef _CODEC_
#define _CODEC_

#include <Audio.h>
#include <CComplex.h>


typedef struct codec_readings_struct {
  float v_rms;
  float i_rms;
  float v_mean;
  float i_mean;
  float phase;
  float v_peak;
  float i_peak;
  bool v_clipping;
  bool i_clipping;
  uint32_t time;
  float i_I_mean;
  float i_Q_mean;
  float v_I_mean;
  float v_Q_mean;
  float i_angle;
  float v_angle;
  Complex gain;
  
} codec_readings_t;

extern bool codecDataAvailable;
extern codec_readings_t codecReadings;
extern uint32_t codecBlocksToAnalyze;

void codecInit();
void codecSetOutputFrequency(float frequency);
void codecSetOutputAmplitude(float amplitude);
bool codecIsClipping(float peaklevel);
void codecResetReadings();
void codecSetReadingsUntilValid(uint8_t num);
void codecAverageReadings();
float codecGetFrequency();
void codecSetBlocksToAnalyze(uint16_t avg = 10);
void codecBlockingMeasure();


#endif
