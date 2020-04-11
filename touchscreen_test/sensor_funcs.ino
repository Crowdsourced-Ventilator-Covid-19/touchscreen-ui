/*
 * This is a dummy placeholder for taking a pressure or flow measurement
 */

double readPressure() {
  double x = millis() / 1000.0;
  double y = 15 * (sin(x*2) + 1);
  //  Graph(tft, x, y, 40, 140, 320, 120, 0, 15, 1, -5, 45, 5, "Pressure", "", "cmH2o", DKBLUE, RED, YELLOW, WHITE, BLACK, display1);
  return y;
}

double readFlow() {
  double x = millis() / 1000.0;
  return 150 * sin(x*2);
}
