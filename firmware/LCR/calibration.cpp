#include "calibration.h"
#include "constants.h"
#include "codec.h"
#include "autorange.h"
#include "ILI9341_t3n.h"
#include "display.h"

probe_type_t current_probes = CLIP_PROBES;
CalibrationPoint calibration_data;
uint8_t num_cal_points = 0;


CalibrationPoint cal_array_raw[MAX_CAL_POINTS];
CalibrationPoint cal_array[MAX_CAL_POINTS];
Complex i_pga_dc[PGA_GAIN_NUM];
Complex v_pga_dc[PGA_GAIN_NUM];



void printCalibrationPoint(CalibrationPoint& cal_data) {

  
  logger.setCursor(10, 10, 9);

  if (num_cal_points == 0) {
    logger.print("No calibration data");
    return;
  }
  
  logger.print("Printing calibration data for: ");
  logger.print(cal_data.frequency);
  logger.println("Hz");

  logger.println("Voltage PGA Gains: ");
  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    logger.print("  ");
    logger.println(cal_data.v_pga_gain[i]);
    //logger.print(", ");
  }
  logger.println();
  logger.println("Current PGA Gains: ");
  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    logger.print("  ");
    logger.println(cal_data.i_pga_gain[i]);
  }
  logger.println();
  logger.println("TIA Gains: ");
  for (uint8_t i=0; i<LCR_RANGE_NUM; i++) {
    logger.print("  ");
    logger.println(cal_data.tia_gain[i]);
  }
  logger.println();
  logger.print("Probe Zs: ");
  logger.println(cal_data.probe_Zs);
  logger.print("Probe Zp: ");
  logger.println(cal_data.probe_Zp);
}



CalibrationPoint averageCalibrationPoints(CalibrationPoint& cal_low, CalibrationPoint& cal_high, float freq) {
  CalibrationPoint cal_average;
  Complex weight_low((cal_high.frequency - freq) / (cal_high.frequency - cal_low.frequency), 0);
  Complex weight_high((freq - cal_low.frequency) / (cal_high.frequency - cal_low.frequency), 0);

  cal_average.frequency = freq;

  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    cal_average.v_pga_gain[i] = weight_low*cal_low.v_pga_gain[i] + weight_high*cal_high.v_pga_gain[i];
    cal_average.i_pga_gain[i] = weight_low*cal_low.i_pga_gain[i] + weight_high*cal_high.i_pga_gain[i];
  }
  
  for (uint8_t i=0; i<LCR_RANGE_NUM; i++) {
    cal_average.tia_gain[i] = weight_low*cal_low.tia_gain[i] + weight_high*cal_high.tia_gain[i];
  }

  cal_average.probe_Zs = weight_low*cal_low.probe_Zs + weight_high*cal_high.probe_Zs;
  cal_average.probe_Zp = weight_low*cal_low.probe_Zp + weight_high*cal_high.probe_Zp;
    
  return cal_average;
}

void loadCalibrationPoint(float freq) {
  uint8_t lower_i = 0;
  
  for (uint8_t i=0; i<num_cal_points; i++) {
    float point_freq = cal_array[i].frequency;

    //I don't actually think this is necessary but check approximate equality just in case
    if (abs(freq - point_freq) < 0.1) {
      calibration_data = cal_array[i];
      return;
    }

    if (point_freq < freq) {
      lower_i = i;
    }   
  }

  if (lower_i == 0 && freq < cal_array[0].frequency) {
    calibration_data = cal_array[0];
    return;
  }

  if (lower_i == num_cal_points - 1) {
    calibration_data = cal_array[num_cal_points - 1];
    return;
  }

  calibration_data = averageCalibrationPoints(cal_array[lower_i], cal_array[lower_i + 1], freq);
}


//Save an array of calibration points to the SD card. Will return true if successful
bool saveCalibration() {

  const char* filename;
  if (current_probes == CLIP_PROBES) {
    filename = CAL_FILE;
  } else {
    filename = CAL_FILE_TWEEZERS;
  }
  
  Serial.println("Starting calibration data save");
  
  if (!SD.begin(CHIP_SELECT)) {Serial.println("SD.begin failed"); return false; }
  Serial.println("SD.begin ok");
  
  SD.remove(filename);
  
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {Serial.println("File open failed"); return false; }

  file.write((const uint8_t*)&CAL_MAGIC,   sizeof(CAL_MAGIC));
  file.write((const uint8_t*)&CAL_VERSION, sizeof(CAL_VERSION));
  file.write((const uint8_t*)&num_cal_points,   sizeof(num_cal_points));
  
  // Write structs
  size_t bytes_written = file.write((const uint8_t*)cal_array, sizeof(CalibrationPoint) * num_cal_points);
  file.close();

  //Verify all structs written
  bool res = (bytes_written == sizeof(CalibrationPoint) * (size_t)num_cal_points);

  if (res) {
    Serial.print(num_cal_points);
    Serial.println(" calibration points saved");
  } else {
    Serial.println("Save failed");
  }

  return res;
}

void saveCalibrationWrapper(float _) {
  board.tft.fillScreen(ILI9341_BLACK);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreen();
    
  bool saved = saveCalibration();
  
  if (saved) {
    String s = "Save of " + String(num_cal_points) + " points successful";
    userPromptText(s);
    board.tft.waitUpdateAsyncComplete();
    board.tft.updateScreen();
  } else {
    userPromptText("Save failed! Check SD card");
    board.tft.waitUpdateAsyncComplete();
    board.tft.updateScreen();
  }

  board.waitForInput();
}


//Load cal_array array with caibration points from the calibration file
//Will return the number of points read into the array
uint8_t loadCalibration() {

  const char* filename;
  if (current_probes == CLIP_PROBES) {
    filename = CAL_FILE;
  } else {
    filename = CAL_FILE_TWEEZERS;
  }

  num_cal_points = 0;
  
  Serial.println("Starting calibration data load");
  
  if (!SD.begin(CHIP_SELECT)) {Serial.println("SD.begin failed"); return 0; }
  Serial.println("SD.begin ok");
  
  File file = SD.open(filename, FILE_READ);
  if (!file) { Serial.println("Calibration file not found"); return 0; }
  Serial.println("Calibration file found");

  uint32_t magic;
  uint8_t version;
  uint8_t count;
  file.read((uint8_t*)&magic,   sizeof(magic));
  file.read((uint8_t*)&version, sizeof(version));
  file.read((uint8_t*)&count,   sizeof(count));

  //Verify the file is calibration data with the correct version and is not too long
  if (magic != CAL_MAGIC)     { file.close(); Serial.println("Incorrect magic number"); return 0; }
  if (version != CAL_VERSION) { file.close(); Serial.println("Incorrect cal version"); return 0; }
  if (count > MAX_CAL_POINTS) { file.close(); Serial.println("Too many cal points"); return 0; }

  size_t bytes_read = file.read((uint8_t*)cal_array_raw, sizeof(CalibrationPoint) * count);
  file.close();

  if (bytes_read != sizeof(CalibrationPoint) * (size_t)count) return 0;

  num_cal_points = count;

  //So there is a vtable pointed inside the Complex class and the direct read I do to load the cal array copies these pointers
  //But they need to be updated because following them will crash the program. Easiest way I found was to just copy into a new array, which does not copy these pointers...
  //I could also have just written my own print function for Complex though
  //TBH I don't really understand the issue, I just know this fixes it and I have plenty of RAM
  for (uint8_t i=0; i<num_cal_points; i++) {
    cal_array[i] = cal_array_raw[i];
  }
  
  return count;
}

void calibrateIPGA_Point(CalibrationPoint& cal_data) {

  //Wipe existing PGA gain values
  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    calibration_data.i_pga_gain[i].polar(1, 0);
  }
  
  cal_data.i_pga_gain[0].polar(1, 0);
  board.setPGAGainI(0);
  board.setLCRRange(LCR_RANGE_100);
  codecSetOutputFrequency(cal_data.frequency);
  codecSetOutputAmplitude(PGA_CAL_AMPLITUDE);
  codecBlockingMeasure();
  float baseline_gain = codecReadings.i_rms;
  float baseline_phase = codecReadings.i_angle;
  
  for (uint8_t i=1; i<PGA_GAIN_NUM; i++) {
    board.setPGAGainI(i);
    codecBlockingMeasure();
    
    float high_gain = codecReadings.i_rms;
    float high_phase = codecReadings.i_angle;
   
    cal_data.i_pga_gain[i].polar(high_gain/baseline_gain, high_phase-baseline_phase);
  }
}

//
void calibrateVPGA_Point(CalibrationPoint& cal_data) {
  
  //Wipe existing PGA gain values
  for (uint8_t i=0; i<PGA_GAIN_NUM; i++) {
    calibration_data.v_pga_gain[i].polar(1, 0);
  }

  cal_data.v_pga_gain[0].polar(1, 0);
  
  board.setPGAGainV(0);
  board.setLCRRange(LCR_RANGE_100K);
  codecSetOutputFrequency(cal_data.frequency);
  codecSetOutputAmplitude(PGA_CAL_AMPLITUDE);
  codecBlockingMeasure();
  float baseline_gain = codecReadings.v_rms;
  float baseline_phase = codecReadings.v_angle;
  
  for (uint8_t i=1; i<PGA_GAIN_NUM; i++) {
    board.setPGAGainV(i);
    codecBlockingMeasure();
    
    float high_gain = codecReadings.v_rms;
    float high_phase = codecReadings.v_angle;
   
    cal_data.v_pga_gain[i].polar(high_gain/baseline_gain, high_phase-baseline_phase);
  }
}


/*
//Given a DC gain, a frequency to evaluate at, and a gain-bandwidth-product, calculate the gain at the specified frequency
Complex calculatePGAGain(Complex dc_gain, float freq, float gbwp) {
  float fc = gbwp / dc_gain.modulus();
  Complex gain;
  gain.polar(1 / sqrt(1 + pow(freq/fc, 2)), -1*atan(freq / fc));
  return dc_gain * gain;
}
*/


/*
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
*/

/*
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
*/


void calibrateIPGA() {

  board.tft.fillScreen(ILI9341_BLACK);
  logger.setCursor(10, 15, 9);
  logger.println_large("IPGA calibration");
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
  
  //getIPGA_DC();

  for (uint8_t i=0; i<num_cal_points; i++) {
    float freq = cal_array[i].frequency;

    calibrateIPGA_Point(cal_array[i]);
      
    logger.print(freq);
    logger.print("Hz: ");
    for (uint8_t j=0; j<PGA_GAIN_NUM; j++) {
      //cal_array[i].i_pga_gain[j] = calculatePGAGain(i_pga_dc[j], freq, I_PGA_GBWP);
      logger.print(cal_array[i].i_pga_gain[j].modulus());
      if (j != PGA_GAIN_NUM - 1) logger.print(", ");
      board.tft.waitUpdateAsyncComplete();
      board.tft.updateScreenAsync();
    }
    logger.println();
  }
}

void calibrateVPGA() {

  board.tft.fillScreen(ILI9341_BLACK);
  logger.setCursor(10, 15, 9);
  logger.println_large("VPGA calibration");
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
  
  //getIPGA_DC();

  for (uint8_t i=0; i<num_cal_points; i++) {
    float freq = cal_array[i].frequency;

    calibrateVPGA_Point(cal_array[i]);
      
    logger.print(freq);
    logger.print("Hz: ");
    for (uint8_t j=0; j<PGA_GAIN_NUM; j++) {
      //cal_array[i].i_pga_gain[j] = calculatePGAGain(i_pga_dc[j], freq, I_PGA_GBWP);
      //logger.print(cal_array[i].v_pga_gain[j]);
      logger.print(cal_array[i].v_pga_gain[j].modulus());
      if (j != PGA_GAIN_NUM - 1) logger.print(", ");
      board.tft.waitUpdateAsyncComplete();
      board.tft.updateScreenAsync();
    }
    logger.println();
  }
}


void calibrateLCRRange_Point(CalibrationPoint& cal_data, uint8_t range, float test_R) {
  Complex R(test_R, 0);
  board.setLCRRange(range);
  codecSetOutputFrequency(cal_data.frequency);
  codecSetOutputAmplitude(MAX_DAC_AMPLITUDE);
  blockingAutorangeMeasure();
  cal_data.tia_gain[range] = R / codecReadings.gain;
}


void calibrateLCRRange(uint8_t range, float test_R) {
  String R_text;
  
  board.tft.fillScreen(ILI9341_BLACK);
  logger.setCursor(10, 15, 9);
  float R = RANGE_RESISTOR[range];
  if (R >= 1000) {
    R_text = String(R/1000.0, 0) + "kOhm";
  } else {
    R_text = String(R, 0) + "Ohm";
  }
  
  logger.println_large("TIA calibration: " + R_text);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
  
  for (uint8_t i=0; i<num_cal_points; i++) {
    calibration_data = cal_array[i];
    calibrateLCRRange_Point(cal_array[i], range, test_R);
    logger.print(cal_array[i].frequency);
    logger.print("Hz: ");
    logger.println(cal_array[i].tia_gain[range]);
    board.tft.waitUpdateAsyncComplete();
    board.tft.updateScreenAsync();
  }
}


void calibrateShort_Point(CalibrationPoint& cal_data) {
  board.setLCRRange(LCR_RANGE_100);
  codecSetOutputFrequency(cal_data.frequency);
  codecSetOutputAmplitude(MAX_DAC_AMPLITUDE);
  blockingAutorangeMeasure();
  cal_data.probe_Zs = codecReadings.gain * cal_data.tia_gain[LCR_RANGE_100];
}

void calibrateShort() {
  board.tft.fillScreen(ILI9341_BLACK);
  logger.setCursor(10, 15, 9);
  logger.println_large("Short calibration");
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
  
  for (uint8_t i=0; i<num_cal_points; i++) {
    float freq = cal_array[i].frequency;
    calibration_data = cal_array[i];
    calibrateShort_Point(cal_array[i]);
    logger.print(freq);
    logger.print("Hz: ");
    logger.println(cal_array[i].probe_Zs);
    board.tft.waitUpdateAsyncComplete();
    board.tft.updateScreenAsync();
  }
}

void calibrateOpen_Point(CalibrationPoint& cal_data) {
  board.setLCRRange(LCR_RANGE_100K);
  codecSetOutputFrequency(cal_data.frequency);
  codecSetOutputAmplitude(MAX_DAC_AMPLITUDE);
  blockingAutorangeMeasure();
  cal_data.probe_Zp = codecReadings.gain * cal_data.tia_gain[LCR_RANGE_100K];
}

void calibrateOpen() {
  board.tft.fillScreen(ILI9341_BLACK);
  logger.setCursor(10, 15, 9);
  logger.println_large("Open calibration");
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
  
  for (uint8_t i=0; i<num_cal_points; i++) {
    float freq = cal_array[i].frequency;
    calibration_data = cal_array[i];
    calibrateOpen_Point(cal_array[i]);
    logger.print(freq);
    logger.print("Hz: ");
    logger.println(cal_array[i].probe_Zp);
    board.tft.waitUpdateAsyncComplete();
    board.tft.updateScreenAsync();
  }
}

//Initial TIA calibration includes errors due to probes + parasitics
//Use estimates of these errors to correct TIA gain
void correctTIAGain() {
  Complex R;
  Complex R_corrected;
  
  Serial.println("Correcting TIA gain");
  
  for (uint8_t i=0; i<num_cal_points; i++) {
    float freq = cal_array[i].frequency;
    Serial.print(freq);
    Serial.print("Hz: ");
    
    for (uint8_t j=0; j<LCR_RANGE_NUM; j++) {
      R.set(RANGE_CAL_RESISTOR[j], 0);
      R_corrected = ((R + cal_array[i].probe_Zs).reciprocal() + cal_array[i].probe_Zp.reciprocal()).reciprocal();
      cal_array[i].tia_gain[j] *= R_corrected / R;
      Serial.print(cal_array[i].tia_gain[j]);
      Serial.print(", ");
    }
    Serial.println();
  }
}


void correctTIAGain_Point(CalibrationPoint& cal_data) {
  Complex R;
  Complex R_corrected;
  
  Serial.println("Correcting TIA gain");
  
  float freq = cal_data.frequency;
  Serial.print(freq);
  Serial.print("Hz: ");
  
  for (uint8_t j=0; j<LCR_RANGE_NUM; j++) {
    R.set(RANGE_CAL_RESISTOR[j], 0);
    R_corrected = ((R + cal_data.probe_Zs).reciprocal() + cal_data.probe_Zp.reciprocal()).reciprocal();
    cal_data.tia_gain[j] *= R_corrected / R;
    Serial.print(cal_data.tia_gain[j]);
    Serial.print(", ");
  }
}


void dispCalHeader(String s) {
  board.tft.fillScreen(ILI9341_BLACK);
  logger.setCursor(10, 15, 9);
  logger.println_large(s);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreen();
}

void calibrateProbes(float _) {

  float amp = codecGetAmplitude();
  float freq = codecGetFrequency();

  board.tft.fillScreen(ILI9341_BLACK);
  userPromptText("Short Probes");
  
  board.waitForInput();
  delay(200);
  calibrateShort();
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  userPromptText("Open Probes");

  board.waitForInput();
  delay(200);
  calibrateOpen();

  board.setPGAGainV(PGA_GAIN_1);
  board.setPGAGainI(PGA_GAIN_1);
  codecSetOutputAmplitude(amp);
  codecSetOutputFrequency(freq);
  loadCalibrationPoint(freq);
}


//Note that the parameter "_" is unused. The calibration is done directly on "calibration_data"
void calibrateProbes_Point(float _) {
  
  float amp = codecGetAmplitude();
  
  board.tft.fillScreen(ILI9341_BLACK);
  userPromptText("Short Probes");
  
  board.waitForInput();
  delay(200);

  dispCalHeader("Short calibration");

  calibrateShort_Point(calibration_data);

  
  logger.print(calibration_data.frequency);
  logger.print("Hz: ");
  logger.println(calibration_data.probe_Zs);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreen();

    
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  userPromptText("Open Probes");
  
  board.waitForInput();
  delay(200);

  dispCalHeader("Open calibration");

  calibrateOpen_Point(calibration_data);

  //board.buzzer.runBuzzerBlocking(4, 10, 50);

  board.setPGAGainV(PGA_GAIN_1);
  board.setPGAGainI(PGA_GAIN_1);
  codecSetOutputAmplitude(amp);
}


//Calibrate all systems using the defined frequency list
void calibrateAll(float _) {

  float amp = codecGetAmplitude();
  float freq = codecGetFrequency();

  //Initialize calibration array with the list of calibration frequencies
  num_cal_points = CAL_FREQ_COUNT;
  for (uint8_t i=0; i<CAL_FREQ_COUNT; i++) {
    cal_array[i].frequency = cal_frequencies[i];
  }

  board.tft.fillScreen(ILI9341_BLACK);
  userPromptText("Connect 10kOhm Resistor");
  
  board.waitForInput();
  delay(200);
  
  calibrateIPGA();
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  userPromptText("Connect 1kOhm Resistor");
  board.waitForInput();
  delay(200);
  
  calibrateVPGA();

  calibrateLCRRange(LCR_RANGE_100, RANGE_CAL_RESISTOR[LCR_RANGE_100]);
  calibrateLCRRange(LCR_RANGE_1K, RANGE_CAL_RESISTOR[LCR_RANGE_1K]);
  
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  userPromptText("Connect 10kOhm Resistor");
  board.waitForInput();
  delay(200);

  calibrateLCRRange(LCR_RANGE_10K, RANGE_CAL_RESISTOR[LCR_RANGE_10K]);
  calibrateLCRRange(LCR_RANGE_100K, RANGE_CAL_RESISTOR[LCR_RANGE_100K]);
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  calibrateProbes();

  correctTIAGain();

  board.setPGAGainV(PGA_GAIN_1);
  board.setPGAGainI(PGA_GAIN_1);
  codecSetOutputAmplitude(amp);
  codecSetOutputFrequency(freq);
  loadCalibrationPoint(freq);

  board.buzzer.runBuzzerBlocking(4, 10, 50);

}


//Calibrate all systems for a given frequency
//Note that the parameter "_" is unused. The calibration is done directly on "calibration_data"
void calibrateAll_Point(float _) {
  float amp = codecGetAmplitude();

  board.tft.fillScreen(ILI9341_BLACK);
  userPromptText("Connect 10kOhm Resistor");
  board.waitForInput();
  delay(200);

  dispCalHeader("IPGA calibration");
  
  calibrateIPGA_Point(calibration_data);

  logger.print(calibration_data.frequency);
  logger.print("Hz: ");
  for (uint8_t j=0; j<PGA_GAIN_NUM; j++) {
    logger.print(calibration_data.i_pga_gain[j].modulus());
    if (j != PGA_GAIN_NUM - 1) logger.print(", ");
    board.tft.waitUpdateAsyncComplete();
    board.tft.updateScreenAsync();
  }
    
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  userPromptText("Connect 1kOhm Resistor");
  board.waitForInput();
  delay(200);

  dispCalHeader("VPGA calibration");
  
  calibrateVPGA_Point(calibration_data);

  for (uint8_t j=0; j<PGA_GAIN_NUM; j++) {
    logger.print(calibration_data.v_pga_gain[j].modulus());
    if (j != PGA_GAIN_NUM - 1) logger.print(", ");
    board.tft.waitUpdateAsyncComplete();
    board.tft.updateScreenAsync();
  }

  delay(2000);

  dispCalHeader("TIA calibration: 100Ohm");
  calibrateLCRRange_Point(calibration_data, LCR_RANGE_100, RANGE_CAL_RESISTOR[LCR_RANGE_100]);
  logger.print(calibration_data.frequency);
  logger.print("Hz: ");
  logger.println(calibration_data.tia_gain[LCR_RANGE_100]);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();

  delay(2000);
  
  dispCalHeader("TIA calibration: 1kOhm");
  calibrateLCRRange_Point(calibration_data, LCR_RANGE_1K, RANGE_CAL_RESISTOR[LCR_RANGE_1K]);
  logger.print(calibration_data.frequency);
  logger.print("Hz: ");
  logger.println(calibration_data.tia_gain[LCR_RANGE_1K]);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();
  
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  userPromptText("Connect 10kOhm Resistor");
  board.waitForInput();
  delay(200);

  dispCalHeader("TIA calibration: 10kOhm");
  calibrateLCRRange_Point(calibration_data, LCR_RANGE_10K, RANGE_CAL_RESISTOR[LCR_RANGE_10K]);
  logger.print(calibration_data.frequency);
  logger.print("Hz: ");
  logger.println(calibration_data.tia_gain[LCR_RANGE_10K]);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();

  delay(2000);

  dispCalHeader("TIA calibration: 100kOhm");
  calibrateLCRRange_Point(calibration_data, LCR_RANGE_100K, RANGE_CAL_RESISTOR[LCR_RANGE_100K]);
  logger.print(calibration_data.frequency);
  logger.print("Hz: ");
  logger.println(calibration_data.tia_gain[LCR_RANGE_100K]);
  board.tft.waitUpdateAsyncComplete();
  board.tft.updateScreenAsync();

  delay(2000);
  
  board.buzzer.runBuzzerBlocking(4, 10, 50);

  calibrateProbes_Point();

  correctTIAGain_Point(calibration_data);
  
  board.setPGAGainV(PGA_GAIN_1);
  board.setPGAGainI(PGA_GAIN_1);
  codecSetOutputAmplitude(amp);
}
