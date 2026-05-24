#include "lcr_func.h"
#include "calibration.h"
#include "codec.h"

float _curr_frequency;
float _curr_amplitude;

void setLCRFrequency(float freq) {
  loadCalibrationPoint(freq);
  codecSetOutputFrequency(freq);
  printCalibrationPoint(calibration_data);
  _curr_frequency = freq;
}

void setLCRAmplitude(float amp) {
  float scaled_amp = amp / DAC_OUTPUT_VPP;
  scaled_amp = min(MAX_DAC_AMPLITUDE, scaled_amp); 
  codecSetOutputAmplitude(scaled_amp);
  _curr_amplitude = amp;
}

float getLCRFrequency() {
  return _curr_frequency;
}

float getLCRAmplitude() {
  return _curr_amplitude;
}

Complex calculateZ() {
  Complex Z_meas = codecReadings.gain * calibration_data.tia_gain[board.getLCRRange()];
  return calibration_data.probe_Zp*Z_meas / (calibration_data.probe_Zp - Z_meas) - calibration_data.probe_Zs;
}

float getQ(Complex Z, float freq) {
  return abs(Z.imag() / Z.real());
}

float getD(Complex Z, float freq) {
  return abs(Z.real() / Z.imag());
}

float getPhasorAngle(Complex Z, float freq) {
  return Z.phase() * 180.0 / M_PI;
}

float getPhasorMag(Complex Z, float freq) {
  return Z.modulus();
}

float getRs(Complex Z, float freq) {
  return max(0, Z.real());
}

float getCs(Complex Z, float freq) {
  return max(0, -1 / (2*M_PI*freq*Z.imag()) );
}

float getLs(Complex Z, float freq) {
  return max(0, Z.imag() / (2*M_PI*freq) );
}

float getRp(Complex Z, float freq) {
  float q = getQ(Z, freq);
  return max(0, Z.real() * (q*q + 1));
}

float getCp(Complex Z, float freq) {
  float d = getD(Z, freq);
  return max(0, -1 / (Z.imag() * 2*M_PI*freq * (d*d + 1)));
}

float getLp(Complex Z, float freq) {
  float d = getD(Z, freq);
  return max(0, Z.imag() * (d*d + 1) / (2*M_PI*freq));
}
