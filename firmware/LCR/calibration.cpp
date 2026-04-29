#include "calibration.h"


//Save an array of calibration points to the SD card. Will return true if successful
bool saveCalibration(const CalibrationPoint* cal_array, uint8_t count) {
  SD.remove(CAL_FILE);
  File file = SD.open(CAL_FILE, FILE_WRITE);
  if (!file) return false;

  file.write((const uint8_t*)&CAL_MAGIC,   sizeof(CAL_MAGIC));
  file.write((const uint8_t*)&CAL_VERSION, sizeof(CAL_VERSION));
  file.write((const uint8_t*)&count,   sizeof(count));
  
  // Write structs
  size_t bytes_written = file.write((const uint8_t*)cal_array, sizeof(CalibrationPoint) * count);
  file.close();

  //Verify all structs written
  return bytes_written == sizeof(CalibrationPoint) * (size_t)count;
}


//Load the passed in array with caibration points from the calibration file
//Will return the number of points read into the array
uint8_t loadCalibration(CalibrationPoint* cal_array) {
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
    
    return count;
}
