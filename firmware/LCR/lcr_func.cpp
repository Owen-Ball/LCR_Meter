#include "lcr_func.h"
#include "calibration.h"
#include "codec.h"
#include "autorange.h"

float _curr_frequency;
float _curr_amplitude;

lcr_param_t primary_lcr_param;
lcr_param_t secondary_lcr_param;

float primary_lcr_value;
float secondary_lcr_value;

lcr_param_t *lcr_param_lookup[LCR_FUNC_NUM] = {
  &lcrParamRs,
  &lcrParamRp,
  &lcrParamCs,
  &lcrParamCp,
  &lcrParamLs,
  &lcrParamLp,
  &lcrParamZMag,
  &lcrParamZPhase,
  &lcrParamQ,
  &lcrParamD,
};

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


void runLCR() {
  codecAverageReadings();
  if (codecDataAvailable) {
    primary_lcr_value = primary_lcr_param.value(calculateZ(), _curr_frequency);
    secondary_lcr_value = secondary_lcr_param.value(calculateZ(), _curr_frequency);
    bool gain_ranged = gainAutorange(false);
    if (!gain_ranged) rangeAutorange(false);
  }
}

void setLCRParams(float f) {
  setLCRParams((int)round(f));
}

void setLCRParams(int index) {
  
  switch(index) {
    
    case 1:
      primary_lcr_param = lcrParamCs;
      secondary_lcr_param = lcrParamRs;
      break;

    case 2:
      primary_lcr_param = lcrParamLs;
      secondary_lcr_param = lcrParamRs;
      break;
      
    default:
      primary_lcr_param = lcrParamCs;
      secondary_lcr_param = lcrParamRs;
      break;
    
  }
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

lcr_param_t lcrParamCs {
  .label = "Cs",
  .unit = "F",
  .resolution = -15,
  .value = &getCs,
};

lcr_param_t lcrParamCp {
  .label = "Cp",
  .unit = "F",
  .resolution = -15,
  .value = &getCp,
};

lcr_param_t lcrParamLs {
  .label = "Ls",
  .unit = "H",
  .resolution = -9,
  .value = &getLs,
};

lcr_param_t lcrParamLp {
  .label = "Lp",
  .unit = "H",
  .resolution = -9,
  .value = &getLp,
};

lcr_param_t lcrParamRs {
  .label = "Rs",
  .unit = "Ohm",
  .resolution = -3,
  .value = &getRs,
};

lcr_param_t lcrParamRp {
  .label = "Rp",
  .unit = "Ohm",
  .resolution = -3,
  .value = &getRp,
};

lcr_param_t lcrParamZMag {
  .label = "|Z|",
  .unit = "Ohm",
  .resolution = -3,
  .value = &getPhasorMag,
};

lcr_param_t lcrParamZPhase {
  .label = "Phi",
  .unit = "o",
  .resolution = 0,
  .value = &getPhasorAngle,
};

lcr_param_t lcrParamD {
  .label = "D",
  .unit = "",
  .resolution = 0,
  .value = &getD,
};

lcr_param_t lcrParamQ {
  .label = "Q",
  .unit = "",
  .resolution = 0,
  .value = &getQ,
};
