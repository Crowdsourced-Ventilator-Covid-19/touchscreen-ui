/*
 * Ventilator touch UI demo
 * Tested using this display: https://www.amazon.com/gp/product/B07NWH47PV/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1
 * Others displays should work as well, if you use the appropriate driver
 * 
 * This code assumes you have a pressure sensor, and a flow sensor.
 * volume measurements are integrated from the flow
 * 
 */


#include <MCUFRIEND_kbv.h>  // use the appropriate driver for your touch TFT display
MCUFRIEND_kbv tft;       
#include <TouchScreen.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49

#define Orientation 1

// -------- input variables -------
int    rr = 15;             // respiratory rate
int    ier = 1;             // I/E ratio  (actually E/I)
int    pmax = 40;           // max pressure (retract piston)
int    peakAlrm = 30;       // indicate pressure alarm
int    platAlrm = 20;       // indicate plateau alarm
int    peepAlrm = 10;       // indicate PEEP alarm
int    tvSet = 300;         // TV set value
int    negTrig = -3;         // negative trigger assist threshold

// -------- main variables -------
String screen;
boolean inspPhase = false;  // phase variable

// measured and processed variables
double peep = 6;            // PEEP measurement
double peak = 20;           // Ppeak measurement
double plat = 18;           // Pplat measurement
double tvMeas = 273;        // measured TV
double p_atmos_hPa = 0;     // atmos pressure in hectoPascals

// temp variables
double tmpP;                // tmp var for pressure
double tmpP_t;              // tmp var for pressure timestamp
double tmpF;                // tmp var for flow
double tmpF_t;              // tmp var for flow timestamp
double tmpTv = 0;           // tmp variable for TV
double tmpPeak = 0;         // tmp variable for peak measurements
double tmpPlat = 0;         // tmp variable for plat measurements
String modVar;              // variable being modified
int    modVal;              // placeholder for new variable
int    omodVal;             // old mod val

// thresholds
double peepError = 0.5;     // thresold for measuring steady state PEEP

// timers
uint32_t sampleTimer = 0;   // sample timer
uint32_t samplePeriod = 50; // sample rate
uint32_t breathTimer;       // next timestamp to start breath
uint32_t ierTimer;          // inspiratory phase duration
uint32_t pAtmosTimer = 0;   // timer to take a new atmos pressure read
uint32_t pAtmosDelay = 1000 * 60UL;  // take new atmos reading every minute

// touchscreen calibration
const int XP=8,XM=A2,YP=A3,YM=9; //ID=0x9341
const int TS_LEFT=94,TS_RT=956,TS_TOP=912,TS_BOT=118;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;
#define MINPRESSURE 200
#define MAXPRESSURE 1000
uint16_t ID;

// these are the only external variables used by the graph function
// it's a flag to draw the coordinate system only on the first call to the Graph() function
// and will mimize flicker
// also create some variables to store the old x and y, if you draw 2 graphs on the same display
// you will need to store ox and oy per each display
boolean display1 = true;
boolean display2 = true;
double ox1 = -999, ox2 = -999, oy1 = -999, oy2 = -999; // Force them to be off screen
double last_t = 999;

void setup() {

  // interrupt driven pressure and flow sample rate to workaround slow screen refresh
  cli();//stop interrupts
  // TIMER 1 for interrupt frequency 100 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 100 Hz increments
  OCR1A = 19999; // = 16000000 / (8 * 100) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 8 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts
  
  //Serial.begin(115000);

  // initialize the TFT
  tft.reset();
  ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1);

  drawMainScreen();
}

void loop() {
  // handler for user interaction
  checkTouch();
  if (screen == "main") {
    updateGraphs();
  }
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt 100Hz
  // read new pressure
  tmpP = readPressure();
  tmpP_t = (millis() % 15000) / 1000.0;

  // save old Flow value and timestamp
  double ot = tmpF_t;
  double of = tmpF;
  // read new flow
  tmpF = readFlow();
  tmpF_t = (millis() % 15000) / 1000.0;

  // deal with overflow condition
  double nt = tmpF_t;
  if (ot > nt) { // wraparound
    nt += 15.0;
  }

  // trapezoidal integration for volume measurement
  tmpTv += (nt - ot) * (of + tmpF) / 2;
}
