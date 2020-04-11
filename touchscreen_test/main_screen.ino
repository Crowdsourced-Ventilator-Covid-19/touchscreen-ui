void drawMainScreen() {
  screen = "main";
  double x, y;
  display1 = true;
  display2 = true;
  tft.fillScreen(BLACK);
  tft.setFont();
  tft.setTextSize(1);
  
  drawMeas("Ppeak", 380, 400, 10, peak, YELLOW);
  drawMeas("Pplat", 380, 400, 70, plat, YELLOW);
  drawMeas("PEEP", 380, 400, 130, peep, YELLOW);
  drawMeas("RR", 380, 400, 190, rr, WHITE);
  drawMeas("TV", 380, 390, 250, tvMeas, GREEN);
}
/*
 * This screen shows the graphs, and measured values
 */


void updateGraphs() {
  double t = (millis() % 15000) / 1000.0;
  if (t < last_t) {
    display1 = true;
    display2 = true;
    tft.fillRect(1, 0, 360, 320, BLACK);
  }
  Graph(tft, tmpP_t, tmpP, 40, 140, 320, 120, 0, 15, 1, -5, 45, 5, "Pressure", "", "cmH2o", DKBLUE, RED, YELLOW, WHITE, BLACK, display1, &ox1, &oy1);
  Graph(tft, tmpF_t, tmpTv, 40, 290, 320, 120, 0, 15, 1, 0, 600, 50, "Volume", "", "ml", DKBLUE, RED, GREEN, WHITE, BLACK, display2, &ox2, &oy2);
  last_t = t;
}

/*
  updateMeas(400, 10, peak, YELLOW);
  updateMeas(400, 70, plat, YELLOW);
  updateMeas(400, 130, peep, YELLOW);
  updateMeas(400, 190, rr, WHITE);
  updateMeas(390, 250, tvMeas, GREEN);
*/

// draw measurement info
void drawMeas(String label, int labelx, int valuex, int y, int value, unsigned int vcolor) {
  tft.setFont();
  tft.setTextSize(1);
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(labelx, y);
  tft.println(label);
  updateMeas(valuex, y, value, vcolor);
}

// update the measurement number
void updateMeas(int x, int y, int value, unsigned int vcolor) {
  tft.fillRect(x, y + 10, 100, 40, BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSansBold24pt7b);
  tft.setTextColor(vcolor, BLACK);
  tft.setCursor(x, y + 50);
  tft.println(value);
}
