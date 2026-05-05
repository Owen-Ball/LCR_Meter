#include "calibration.h"
#include "constants.h"
#include "codec.h"


CalibrationPoint calibration_data;
uint8_t num_cal_points = 10;
CalibrationPoint cal_array[MAX_CAL_POINTS];
Complex i_pga_dc[PGA_GAIN_NUM];
Complex v_pga_dc[PGA_GAIN_NUM];

bool loadCalibrationPoint() {
  calibration_data.v_pga_gain[PGA_GAIN_1].polar(1, 0);
  calibration_data.v_pga_gain[PGA_GAIN_5].polar(5, 0);
  calibration_data.v_pga_gain[PGA_GAIN_25].polar(25, 0);
  calibration_data.v_pga_gain[PGA_GAIN_100].polar(100, 0);

  calibration_data.i_pga_gain[PGA_GAIN_1].polar(1, 0);
  calibration_data.i_pga_gain[PGA_GAIN_5].polar(1, 0);
  calibration_data.i_pga_gain[PGA_GAIN_25].polar(1, 0);
  calibration_data.i_pga_gain[PGA_GAIN_100].polar(1, 0);

  calibration_data.tia_gain[LCR_RANGE_100].polar(100, 0);
  calibration_data.tia_gain[LCR_RANGE_1K].polar(1000, 0);
  calibration_data.tia_gain[LCR_RANGE_10K].polar(10000, 0);
  calibration_data.tia_gain[LCR_RANGE_100K].polar(100000, 0);
  
  return true;
}


//Save an array of calibration points to the SD card. Will return true if successful
bool saveCalibration() {
  SD.remove(CAL_FILE);
  File file = SD.open(CAL_FILE, FILE_WRITE);
  if (!file) return false;

  file.write((const uint8_t*)&CAL_MAGIC,   sizeof(CAL_MAGIC));
  file.write((const uint8_t*)&CAL_VERSION, sizeof(CAL_VERSION));
  file.write((const uint8_t*)&num_cal_points,   sizeof(num_cal_points));
  
  // Write structs
  size_t bytes_written = file.write((const uint8_t*)cal_array, sizeof(CalibrationPoint) * num_cal_points);
  file.close();

  //Verify all structs written
  return bytes_written == sizeof(CalibrationPoint) * (size_t)num_cal_points;
}


//Load the passed in array with caibration points from the calibration file
//Will return the number of points read into the array
uint8_t loadCalibration() {
  File file = SD.open(CAL_FILE, FILE_READ);
  if (!file) return 0;

  uint32_t magic;
  uint16_t version;
  uint8_t count;
  file.read((uint8_t*)&magic,   sizeof(magic));
  file.read((uint8_t*)&version, sizeof(version));
  file.read((uint8_t*)&count,   sizeof(count));

  //Verify the file is calibration data with the correct version and is not too long
  if (magic != CAL_MAGIC)     { file.close(); return 0; }
  if (version != CAL_VERSION) { file.close(); return 0; }
  if (count > MAX_CAL_POINTS) { file.close(); return 0; }

  size_t bytes_read = file.read((uint8_t*)cal_array, sizeof(CalibrationPoint) * count);
  file.close();

  if (bytes_read != sizeof(CalibrationPoint) * (size_t)count) return 0;

  num_cal_points = count;
  return count;
}

void calibrateIPGA_Full(CalibrationPoint& cal_data) {

  calibration_data = cal_data;
  
  //Wipe existing PGA gain values
  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    cal_data.i_pga_gain[i].polar(1, 0);
  }

  float input_amplitude = MAX_DAC_AMPLITUDE;
  board.setPGAGainI(0);
  board.setLCRRange(LCR_RANGE_100);
  codecSetOutputAmplitude(input_amplitude);
  codecBlockingMeasure();
  float vpp = codecReadings.i_peak;
  //delay(1000);
  for (uint8_t i=1; i<PGA_GAIN_NUM; i++) {
    board.setPGAGainI(i);
    input_amplitude *= (CODEC_CLIPPING_LEVEL/7.5) / vpp;
    codecSetOutputAmplitude(input_amplitude);
    codecBlockingMeasure();
    //delay(1000);
    while (codecReadings.i_clipping) {
      input_amplitude /= 1.25;
      codecSetOutputAmplitude(input_amplitude);
      codecBlockingMeasure();
      Serial.println(String(i));
      //delay(1000);
    }
    float high_gain = codecReadings.i_rms;
    float high_phase = codecReadings.i_angle;
    vpp = codecReadings.i_peak;
    
    board.decreaseIGain();
    codecBlockingMeasure();
    //delay(1000);
    float low_gain = codecReadings.i_rms;
    float low_phase = codecReadings.i_angle;

    cal_data.i_pga_gain[i].polar(high_gain/low_gain, high_phase-low_phase);
  }
  Serial.println(cal_data.i_pga_gain[3]);
  delay(100);
  
}

//Given a DC gain, a frequency to evaluate at, and a gain-bandwidth-product, calculate the gain at the specified frequency
Complex calculatePGAGain(Complex dc_gain, float freq, float gbwp) {
  float fc = gbwp / dc_gain.modulus();
  Complex gain;
  gain.polar(1 / sqrt(1 + pow(freq/fc, 2)), -1*atan(freq / fc));
  return dc_gain * gain;
}

//Measure the Current PGA at the lowest frequency
void getIPGA_DC() {
  
  codecSetOutputFrequency(LCR_MIN_FREQUENCY);
  
  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    calibration_data.i_pga_gain[i].polar(1, 0);
  }
  
  float input_amplitude = MAX_DAC_AMPLITUDE;
  board.setPGAGainI(0);
  board.setLCRRange(LCR_RANGE_100);
  codecSetOutputAmplitude(input_amplitude);
  codecBlockingMeasure();
  float vpp = codecReadings.i_peak;

  for (uint8_t i=1; i<PGA_GAIN_NUM; i++) {
    board.setPGAGainI(i);
    input_amplitude *= (CODEC_CLIPPING_LEVEL/7.5) / vpp;
    codecSetOutputAmplitude(input_amplitude);
    codecBlockingMeasure();
    
    while (codecReadings.i_clipping) {
      input_amplitude /= 1.25;
      codecSetOutputAmplitude(input_amplitude);
      codecBlockingMeasure();
    }
    
    float high_gain = codecReadings.i_rms;
    float high_phase = codecReadings.i_angle;
    
    vpp = codecReadings.i_peak;
    
    board.decreaseIGain();
    codecBlockingMeasure();
 
    float low_gain = codecReadings.i_rms;
    float low_phase = codecReadings.i_angle;

    calibration_data.i_pga_gain[i].polar(high_gain/low_gain, high_phase-low_phase);
    i_pga_dc[i] = calibration_data.i_pga_gain[i];
  }
}


//Measure the Current PGA at the lowest frequency
void getVPGA_DC() {
  
  codecSetOutputFrequency(LCR_MIN_FREQUENCY);
  
  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    calibration_data.v_pga_gain[i].polar(1, 0);
  }
  
  float input_amplitude = MAX_DAC_AMPLITUDE;
  board.setPGAGainV(0);
  board.setLCRRange(LCR_RANGE_100);
  codecSetOutputAmplitude(input_amplitude);
  codecBlockingMeasure();
  float vpp = codecReadings.v_peak;

  for (uint8_t i=1; i<PGA_GAIN_NUM; i++) {
    board.setPGAGainV(i);
    input_amplitude *= (CODEC_CLIPPING_LEVEL/7.5) / vpp;
    codecSetOutputAmplitude(input_amplitude);
    codecBlockingMeasure();
    
    while (codecReadings.v_clipping) {
      input_amplitude /= 1.25;
      codecSetOutputAmplitude(input_amplitude);
      codecBlockingMeasure();
    }
    
    float high_gain = codecReadings.v_rms;
    float high_phase = codecReadings.v_angle;
    
    vpp = codecReadings.v_peak;
    
    board.decreaseVGain();
    codecBlockingMeasure();
 
    float low_gain = codecReadings.v_rms;
    float low_phase = codecReadings.v_angle;

    calibration_data.v_pga_gain[i].polar(high_gain/low_gain, high_phase-low_phase);
    v_pga_dc[i] = calibration_data.v_pga_gain[i];
  }
}


void calibrateIPGA() {
  calibration_data = cal_array[0];

  getIPGA_DC();

  for (uint8_t i=0; i<num_cal_points; i++) {
    float freq = cal_array[i].frequency;

    Serial.print(freq);
    Serial.print("Hz: ");
    for (uint8_t j=0; j<PGA_GAIN_NUM; j++) {
      cal_array[i].i_pga_gain[j] = calculatePGAGain(i_pga_dc[j], freq, I_PGA_GBWP);
      Serial.print(cal_array[i].i_pga_gain[j]);
      Serial.print(", ");
    }
    Serial.println();
  }
}


void calibrateVPGA() {
  calibration_data = cal_array[0];

  getVPGA_DC();

  for (uint8_t i=0; i<num_cal_points; i++) {
    float freq = cal_array[i].frequency;

    Serial.print(freq);
    Serial.print("Hz: ");
    for (uint8_t j=0; j<PGA_GAIN_NUM; j++) {
      cal_array[i].v_pga_gain[j] = calculatePGAGain(v_pga_dc[j], freq, V_PGA_GBWP);
      Serial.print(cal_array[i].v_pga_gain[j]);
      Serial.print(", ");
    }
    Serial.println();
  }
}


//Calibrate all systems using the defined freqeuncy list
void calibrateAll() {

  //Initialize calibration array with the list of calibration frequencies
  num_cal_points = CAL_FREQ_COUNT;
  for (uint8_t i=0; i<CAL_FREQ_COUNT; i++) {
    cal_array[i].frequency = cal_frequencies[i];
  }

  calibrateIPGA();

  calibrateVPGA();

  //codecSetOutputFrequency(75000);

  //calibrateIPGA_Full(calibration_data);
}
