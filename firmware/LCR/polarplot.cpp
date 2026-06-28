#include "polarplot.h"
#include "ILI9341_t3n.h"
#include "constants.h"
#include "board.h"
#include "LCR_Fonts/Font5x7FixedMono.h"

int16_t x_mid;
int16_t y_mid;
float pixels_per_div;




void drawAxes() {
  board.tft.drawLine(x_mid - POLARPLOT_SIZE/2, y_mid, x_mid + POLARPLOT_SIZE/2, y_mid, ILI9341_WHITE);
  board.tft.drawLine(x_mid, y_mid - POLARPLOT_SIZE/2, x_mid, y_mid + POLARPLOT_SIZE/2, ILI9341_WHITE);

  board.tft.drawCircle(x_mid, y_mid, int16_t(3*pixels_per_div), 0xdedb);
  board.tft.drawCircle(x_mid, y_mid, int16_t(6*pixels_per_div), 0xdedb);
  board.tft.drawCircle(x_mid, y_mid, int16_t(9*pixels_per_div), 0xdedb);

  board.tft.setTextSize(1);
  board.tft.setTextColor(ILI9341_WHITE);
  board.tft.setFont(&Font5x7FixedMono);
  
  for (int8_t i = POLARPLOT_MIN_EXP + POLARPLOT_LABEL_INTERVAL; i <= POLARPLOT_MAX_EXP; i += POLARPLOT_LABEL_INTERVAL) {
     board.tft.setCursor(x_mid + i*pixels_per_div + 13, y_mid + 6);
     board.tft.print(String(i));
  }

  board.tft.setCursor(x_mid + POLARPLOT_SIZE/2 - 4, y_mid - 4);
  board.tft.print("R");

  board.tft.setCursor(x_mid - 8, y_mid - POLARPLOT_SIZE/2 + 3);
  board.tft.print("X");
  

}

void polarPlotZ(Complex Z) {
  x_mid = POLARPLOT_X_POS + POLARPLOT_SIZE/2;
  y_mid = POLARPLOT_Y_POS + POLARPLOT_SIZE/2;

  pixels_per_div = float(POLARPLOT_SIZE) / (2.0 * (POLARPLOT_MAX_EXP - POLARPLOT_MIN_EXP + 1));

  float mag = log10(Z.modulus());
  mag = min(mag, POLARPLOT_MAX_EXP);
  float angle = Z.phase();

  int16_t p_x = x_mid + int16_t((mag - POLARPLOT_MIN_EXP)*pixels_per_div*cos(angle));
  int16_t p_y = y_mid - int16_t((mag - POLARPLOT_MIN_EXP)*pixels_per_div*sin(angle));

  drawAxes();
  board.tft.fillCircle(p_x, p_y, 2, ILI9341_RED);
  
}
