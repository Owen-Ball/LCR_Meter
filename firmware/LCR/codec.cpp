#include "codec.h"
#include "constants.h"
#include "calibration.h"
#include "audio/analyze_mean.h"
#include "audio/analyze_mean.cpp"
#include "audio/control_cs4272_192k.h"
#include "audio/control_cs4272_192k.cpp"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioSynthWaveform       sine_DAC  ;     //xy=388,281
AudioInputI2Sslave       audioInI2S1;    //xy=570,554
AudioSynthWaveform       squarewave_90;  //xy=572,633
AudioOutputI2Sslave      audioOutI2S1;   //xy=574,298
AudioSynthWaveform       squarewave;     //xy=578,474
AudioAnalyzePeak         peakI;          //xy=838,731
AudioAnalyzePeak         peakV;          //xy=842,385
AudioEffectMultiply      multiplierV_I;  //xy=857,491
AudioEffectMultiply      multiplierV_Q;  //xy=857,539
AudioEffectMultiply      multiplierI_I;  //xy=857,580
AudioEffectMultiply      multiplierI_Q;  //xy=857,620
AudioAnalyzeRMS          analyzeRmsI;    //xy=857,693
AudioAnalyzeMean         analyzeMeanI; //xy=860,769
AudioAnalyzeRMS          analyzeRmsV;    //xy=863,423
AudioAnalyzeMean         analyzeMeanV; //xy=867,345
AudioAnalyzeMean         meanI_I; //xy=994.4999656677246,580.1666822433472
AudioAnalyzeMean         meanV_I; //xy=995,491
AudioAnalyzeMean         meanV_Q;  //xy=995,539
AudioAnalyzeMean         meanI_Q; //xy=996,620
AudioConnection          patchCord1(sine_DAC,    0, audioOutI2S1, 0);
AudioConnection          patchCord20(sine_DAC,   0, audioOutI2S1, 1);
AudioConnection          patchCord2(audioInI2S1, 0, analyzeRmsV, 0);
AudioConnection          patchCord3(audioInI2S1, 0, multiplierV_I, 1);
AudioConnection          patchCord4(audioInI2S1, 0, multiplierV_Q, 1);
AudioConnection          patchCord5(audioInI2S1, 0, peakV, 0);
AudioConnection          patchCord6(audioInI2S1, 0, analyzeMeanV, 0);
AudioConnection          patchCord7(audioInI2S1, 1, analyzeRmsI, 0);
AudioConnection          patchCord8(audioInI2S1, 1, multiplierI_I, 1);
AudioConnection          patchCord9(audioInI2S1, 1, multiplierI_Q, 1);
AudioConnection          patchCord10(audioInI2S1, 1, peakI, 0);
AudioConnection          patchCord11(squarewave_90, 0, multiplierI_Q, 0);
AudioConnection          patchCord12(squarewave_90, 0, multiplierV_Q, 0);
AudioConnection          patchCord13(audioInI2S1, 1, analyzeMeanI, 0);
AudioConnection          patchCord14(squarewave, 0, multiplierV_I, 0);
AudioConnection          patchCord15(squarewave, 0, multiplierI_I, 0);
AudioConnection          patchCord16(multiplierV_I, 0, meanV_I, 0);
AudioConnection          patchCord17(multiplierV_Q, 0, meanV_Q, 0);
AudioConnection          patchCord18(multiplierI_I, 0, meanI_I, 0);
AudioConnection          patchCord19(multiplierI_Q, 0, meanI_Q, 0);
AudioControlCS4272_192k  CS4272;         //xy=592,180


codec_readings_t codecReadings;
bool codecDataAvailable = false;
float _frequency;
float _amplitude;
uint32_t codecBlocksToAnalyze = 150;
bool codecDiscardResults = false;
uint8_t codecReadingsUntilValid = 0;


// Set the phase of the square wave signals and sine signal
void codecResetPhase() {
  AudioNoInterrupts();
  squarewave.phase(0);
  squarewave_90.phase(90);
  sine_DAC.phase(0);
  AudioInterrupts();
}

void codecInit() {
  AudioMemory(100);
  CS4272.enable();

  codecSetOutputFrequency(1000);

  // init sine wave signal
  sine_DAC.amplitude(0.5);
  sine_DAC.begin(WAVEFORM_SINE);
  
  // init squarewave signals
  squarewave.amplitude(1);
  squarewave_90.amplitude(1);
  squarewave.begin(WAVEFORM_SQUARE);
  squarewave_90.begin(WAVEFORM_SQUARE);
  
  codecResetPhase(); 
}


// Set frequency of output signal in Hz.
void codecSetOutputFrequency(float frequency) {
  sine_DAC.frequency(frequency);
  squarewave.frequency(frequency);
  squarewave_90.frequency(frequency);
  codecResetPhase();
  _frequency = frequency;
  codecSetBlocksToAnalyze(10);
  codecResetReadings();
}


// Set the sine wave amplitude in Vpp
void codecSetOutputAmplitude(float amplitude) {
  sine_DAC.amplitude(amplitude);
  codecResetReadings();
  codecResetPhase();
  _amplitude = amplitude;
}

//A rough fit of the CS4272 ADC rolloff for quad mode
float codecADCFreqResponse(float f) {
  f = f / AUDIO_SAMPLE_RATE_EXACT;
  float gain = 0;
  if (f > 0.18) {
    gain = -310*pow(f - 0.18, 3);
  }

  return pow(10, gain/20);
}

// Determine if the codec input is clipping given peak signal
bool codecIsClipping(float peaklevel) {
  return peaklevel > CODEC_CLIPPING_LEVEL;
}

// Set a flag to reset all analyze objects after next available block
void codecResetReadings() {
  codecDiscardResults = true;
  codecDataAvailable = false;
}

void codecSetReadingsUntilValid(uint8_t num) {
  codecReadingsUntilValid = num;
}

float wrapAngle(float angle) {
    angle = fmod(angle, 2 * M_PI);
    if (angle > M_PI)  angle -= 2 * M_PI;
    if (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}


// Process codec audio blocks and store data to struct
void codecAverageReadings() {
  uint32_t analyzedBlocks = meanI_I.available();
  if (analyzedBlocks > codecBlocksToAnalyze) {
    codecDiscardResults = true;
  }

  if (analyzedBlocks == codecBlocksToAnalyze && codecReadingsUntilValid > 0) {
    codecReadingsUntilValid -= 1;
    codecDiscardResults = true;
  }

  if (codecDiscardResults) {
    // All collected data needs to be discarded.
    peakV.read();
    peakI.read();
    analyzeRmsV.read();
    analyzeRmsI.read();
    analyzeMeanV.read();
    analyzeMeanI.read();
    meanI_I.read();
    meanI_Q.read();
    meanV_I.read();
    meanV_Q.read();
    codecDiscardResults = false;
    codecDataAvailable = false;
    return;
  }

  if (analyzedBlocks < codecBlocksToAnalyze) {
    return;
  }
  
  // Enough analyzed data is avaliable. Get readings...
  codecReadings.v_peak = peakV.read() / codecADCFreqResponse(_frequency);
  codecReadings.i_peak = peakI.read() / codecADCFreqResponse(_frequency);
  codecReadings.v_clipping = codecIsClipping(codecReadings.v_peak);
  codecReadings.i_clipping = codecIsClipping(codecReadings.i_peak);
  codecReadings.v_rms = analyzeRmsV.read() / calibration_data.v_pga_gain[board.getPGAGainV()].modulus();
  codecReadings.i_rms = analyzeRmsI.read() / calibration_data.i_pga_gain[board.getPGAGainI()].modulus();
  codecReadings.v_mean = analyzeMeanV.read() / calibration_data.v_pga_gain[board.getPGAGainV()].modulus();
  codecReadings.i_mean = analyzeMeanI.read() / calibration_data.i_pga_gain[board.getPGAGainI()].modulus();
  codecReadings.i_I_mean = meanI_I.read();
  codecReadings.i_Q_mean = meanI_Q.read();
  codecReadings.v_I_mean = meanV_I.read();
  codecReadings.v_Q_mean = meanV_Q.read();
  codecReadings.v_angle = atan2(codecReadings.v_Q_mean, codecReadings.v_I_mean) - calibration_data.v_pga_gain[board.getPGAGainV()].phase();
  codecReadings.i_angle = M_PI + atan2(codecReadings.i_Q_mean, codecReadings.i_I_mean) - calibration_data.i_pga_gain[board.getPGAGainI()].phase();
  codecReadings.phase = wrapAngle(codecReadings.v_angle - codecReadings.i_angle);
  codecReadings.gain.polar(codecReadings.v_rms / codecReadings.i_rms, codecReadings.phase);

  codecDataAvailable = true;
  codecResetPhase();
}


// Returns the currently used output frequency
float codecGetFrequency() {
  return _frequency;
}

// Returns the currently used output amplitude
float codecGetAmplitude() {
  return _amplitude;
}


// Set number of audio blocks to average across
void codecSetBlocksToAnalyze(uint16_t min_periods) {
  codecBlocksToAnalyze = AUDIO_SAMPLE_RATE_EXACT / AUDIO_BLOCK_SAMPLES / LCR_MIN_FREQUENCY * min_periods;
}

//Reset readings and measure until new data is ready
void codecBlockingMeasure() {
  delay(CODEC_SETTING_CHANGE_DELAY);
  codecResetReadings();
  while (!codecDataAvailable) {
    codecAverageReadings();
  }
}
