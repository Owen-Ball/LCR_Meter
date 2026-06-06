#include "lcr_func.h"
#include "calibration.h"
#include "codec.h"
#include "autorange.h"

float _curr_frequency;
float _curr_amplitude;

lcr_param_t primary_lcr_param;
lcr_param_t secondary_lcr_param;

bool auto_param;

float primary_lcr_value;
float secondary_lcr_value;

float resistance_lcr_value;
float reactance_lcr_value;

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


void runAutoParam() {
  Complex Z = calculateZ();
  float Q = getQ(Z, _curr_frequency);
  float X = getXs(Z, _curr_frequency);

  if (Q < 0.1) {
    primary_lcr_param = lcrParamRs;
    secondary_lcr_param = lcrParamQ;
  } else if (Q < 20 && X > 0) {
    primary_lcr_param = lcrParamLs;
    secondary_lcr_param = lcrParamRs;
  } else if (Q < 20 && X < 0) {
    primary_lcr_param = lcrParamCs;
    secondary_lcr_param = lcrParamRs;
  } else if (X > 0) {
    primary_lcr_param = lcrParamLp;
    secondary_lcr_param = lcrParamRp;
  } else if (X < 0) {
    primary_lcr_param = lcrParamCp;
    secondary_lcr_param = lcrParamRp;
  } else {
    primary_lcr_param = lcrParamCs;
    secondary_lcr_param = lcrParamRs;
  }
  
}

void runLCR() {
  codecAverageReadings();
  if (codecDataAvailable) {
    resistance_lcr_value = getRs_signed(calculateZ(), _curr_frequency);
    reactance_lcr_value = getXs(calculateZ(), _curr_frequency);

    if (auto_param) runAutoParam();
    
    primary_lcr_value = primary_lcr_param.value(calculateZ(), _curr_frequency);
    secondary_lcr_value = secondary_lcr_param.value(calculateZ(), _curr_frequency);

    
    bool gain_ranged = gainAutorange(false);
    if (!gain_ranged) rangeAutorange(false);
    codecResetReadings();
  }
}

void setLCRParams(int index) {

  
  switch(index) {

    case 0:
      primary_lcr_param = lcrParamCs;
      secondary_lcr_param = lcrParamRs;
      auto_param = true;
      break;
    case 1:
      primary_lcr_param = lcrParamCs;
      secondary_lcr_param = lcrParamRs;
      auto_param = false;
      break;
    case 2:
      primary_lcr_param = lcrParamLs;
      secondary_lcr_param = lcrParamRs;
      auto_param = false;
      break;
    case 3:
      primary_lcr_param = lcrParamCp;
      secondary_lcr_param = lcrParamRp;
      auto_param = false;
      break;
    case 4:
      primary_lcr_param = lcrParamLp;
      secondary_lcr_param = lcrParamRp;
      auto_param = false;
      break;
    default:
      primary_lcr_param = lcrParamCs;
      secondary_lcr_param = lcrParamRs;
      auto_param = false;
      break;
    
  }
}

void setLCRParams(float f) {
  int i = (int)round(f);
  setLCRParams(i);
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

float getXs(Complex Z, float freq) {
  return Z.imag();
}

float getRs_signed(Complex Z, float freq) {
  return Z.real();
}

lcr_param_t lcrParamCs {
  .label = "Cs",
  .unit = "F",
  .resolution = -12,
  .value = &getCs,
};

lcr_param_t lcrParamCp {
  .label = "Cp",
  .unit = "F",
  .resolution = -12,
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
  .unit = "O",
  .resolution = -3,
  .value = &getRs,
};

lcr_param_t lcrParamRp {
  .label = "Rp",
  .unit = "O",
  .resolution = -3,
  .value = &getRp,
};

lcr_param_t lcrParamZMag {
  .label = "|Z|",
  .unit = "O",
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
