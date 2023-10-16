
/*
  Author: Yash Chaudhary
  Date: 16/10/2023
  NOTES: Ensure TFT LCD setup files have been properly setup
*/

// ILI9488 dimensions: 320 x 480

// include libraries and files
#include "SPI.h"
#include "TFT_eSPI.h"
#include "nose_icon.h"      // bitmap 48 x 48 px 
#include "moon_icon.h"      // bitmap 45 x 45 px
#include "tick_icon.h"      // bitmap 45 x 45 px
#include "warning_icon.h"   // bitmap 45 x 45 px
#include "cross_icon.h"     // bitmap 45 x 45 px
#include "danger_icon.h"    // bitmap 45 x 45 px
#include <SoftwareSerial.h> // default software serial library
#include <TFT_eWidget.h>    // widget library (for graphing)

// software serial configurations
int rxPin = 4;
int txPin = 5;
#define baudrate 9600   // baudrate matches speed of data transmission coming from ArduinoUNO
SoftwareSerial mySerial (rxPin, txPin);

// serial communication data 
String incomingString;
String values[6];

// tft UI library instantiation
TFT_eSPI tft = TFT_eSPI();

// arc gauge color schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

// arc meter configurations
int xpos = 18;
int ypos = 62;
int gap = 15;
int radius = 83;
int reading = 0;

// air quality status enums
// #define AQ_GOOD 0
// #define AQ_FAIR 1
// #define AQ_POOR 2
// #define AQ_DANGER 3

// custom colors
#define TFT_GREY 0x528a
uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21);

// graph configurations
GraphWidget gr = GraphWidget(&tft); 
TraceWidget tr1 = TraceWidget(&gr); // traces are drawn on tft using graph instance

// control execution
static unsigned long last;


//////////////////////////////////////////
/*            Setup Function            */
//////////////////////////////////////////
void setup() {
  
    // start serial monitor
    Serial.begin(9600); 
    while (!Serial);

    // start software serial
    mySerial.begin(baudrate);
    while (!mySerial);
  
    // tft LCD setup
    tft.init();                     // initialise TFT LCD display
    tft.fillScreen(TFT_BLACK);      // set default background color to black
    tft.setRotation(1);             // set LCD to landscape orientation
    tft.setTextColor(TFT_WHITE);    // global text color
    tft.setSwapBytes(true);         // allow use of images (bitmaps)

    // graph initialisations
    gr.createGraph(200, 100, tft.color565(5, 5, 5));   // Graph area is 200 pixels wide, 150 high, dark grey background
    gr.setGraphScale(0.0, 100.0, -50.0, 50.0);         // x scale units is from 0 to 100, y scale units is -50 to 50

    // X grid starts at 0 with lines every 10 x-scale units
    // Y grid starts at -50 with lines every 25 y-scale units
    // blue grid
    gr.setGraphGrid(0.0, 10.0, -50.0, 25.0, TFT_BLUE);


    gr.drawGraph(240, 195);       // Draw empty graph, top left corner at 40,10 on TFT
    tr1.startTrace(TFT_RED);      // Start a trace with using red and another with green
    tr1.addPoint(0.0, 0.0);        // Add points on graph to trace 1 using graph scale factors

    // Get x,y pixel coordinates of any scaled point on graph and ring that point.
    tft.drawCircle(gr.getPointX(50.0), gr.getPointY(0.0), 5, TFT_MAGENTA);

    // Draw the x axis scale
    tft.setTextDatum(TC_DATUM); // Top centre text datum
    tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(-50.0) + 3);
    tft.drawNumber(50, gr.getPointX(50.0), gr.getPointY(-50.0) + 3);
    tft.drawNumber(100, gr.getPointX(100.0), gr.getPointY(-50.0) + 3);

    // Draw the y axis scale
    tft.setTextDatum(MR_DATUM); // Middle right text datum
    tft.drawNumber(-50, gr.getPointX(0.0), gr.getPointY(-50.0));
    tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(0.0));
    tft.drawNumber(50, gr.getPointX(0.0), gr.getPointY(50.0));

    // Restart traces with new colours
    tr1.startTrace(TFT_WHITE);
}


//////////////////////////////////////////
/*          Superloop Function          */
//////////////////////////////////////////
void loop(void) {

    
    // check data in serial bugger
    if(mySerial.available() > 0) {

        // create incomingString of string type and read unitl a newline terminating character is reached
        incomingString = mySerial.readStringUntil('\n');
        // Split the input string into individual values based on commas
        int numValues = split(incomingString, values, ',');
  
        Serial.println(incomingString);
    }

    // pull out return values
    String ppm = values[0];
    String temp = values[1];
    String humidity = values[2];
    String isDay = values[3];
    String date = values[4];
    String time = values[5];

    drawContainers();
    setName();
    setActivity(isDay, date, time);
    setTemperature(temp);
    setHumidity(humidity);

    // air quality meter
    if (ppm == "") {
      reading = 0;
    } else {
      reading = ppm.toInt();
    }
    
    ringMeter(reading,0,999, xpos, ypos, radius,"PPM",GREEN2RED); // Draw analogue meter

    setStatus(reading);

    //delay(5000);
    static uint32_t plotTime = millis();
  static float gx = 0.0, gy = 0.0;
  static float delta = 7.0;

  // Sample periodically
  if (millis() - plotTime >= 100) {
    plotTime = millis();

    // Add a new point on each trace
    tr1.addPoint(gx, gy/2.0);

    // Create next plot point
    gx += 1.0;
    gy += delta;
    if (gy >  70.0) { delta = -7.0; gy =  70.0; }
    if (gy < -70.0) { delta =  7.0; gy = -70.0; }

    // If the end of the graph is reached start 2 new traces
    if (gx > 100.0) {
      gx = 0.0;
      gy = 0.0;

      // Draw empty graph at 40,10 on display
      gr.drawGraph(240, 195);
      // Start new trace
      tr1.startTrace(TFT_GREEN);
    }
  }
}

// function that draws container boundaries
void drawContainers() {

  uint16_t lineColor = TFT_WHITE;

  // outer container with thickness of 2 and background of  
  tft.drawFastHLine(0, 0, 479, lineColor);  // top edge
  tft.drawFastVLine(479, 0, 319, lineColor);  // right edge
  tft.drawFastHLine(0, 319, 479, lineColor);  // bottom edge
  tft.drawFastVLine(0, 0, 319, lineColor);  // left edge

  tft.drawFastHLine(0, 1, 479, lineColor);  // top edge
  tft.drawFastVLine(478, 0, 319, lineColor);  // right edge
  tft.drawFastHLine(0, 318, 479, lineColor);  // bottom edge
  tft.drawFastVLine(1, 0, 319, lineColor);  // left edge

  // header container with thickness of 2
  tft.drawFastHLine(0, 49, 479, lineColor);  // header bottom edge
  tft.drawFastHLine(0, 50, 479, lineColor);  // header bottom edge

  // air quality sensor container with thickness of 2
  tft.drawFastVLine(200, 49, 319, lineColor);  // air quality left edge
  tft.drawFastVLine(199, 49, 319, lineColor);  // air quality left edge

  // air quality box rectangles
  tft.fillRect(4, 53, 50, 10, textColor);
  tft.fillRect(4, 53, 10, 50, textColor);
  tft.fillRect(146, 53, 50, 10, textColor);
  tft.fillRect(186, 53, 10, 50, textColor);
  tft.fillRect(4, 217, 10, 50, textColor);
  tft.fillRect(4, 257, 50, 10, textColor);
  tft.fillRect(186, 217, 10, 50, textColor);
  tft.fillRect(146, 257, 50, 10, textColor);

  // temp sensor, humidity lvl and buzzer freq output divider
  tft.drawFastHLine(200, 185, 479, lineColor);  
  tft.drawFastHLine(200, 184, 479, lineColor);  

  // temp sensor and humidity container divider (thickness of 2)
  tft.drawFastVLine(340, 49, 135, lineColor);  
  tft.drawFastVLine(339, 49, 135, lineColor);  

  // device name boundary
  tft.drawFastHLine(0, 270, 200, lineColor);  
  tft.drawFastHLine(0, 269, 200, lineColor); 
}

// a function that sets the name and bitmap for the project
void setName() {
  tft.pushImage(2,2,48,48,nose_icon);  // display nose bit map
  tft.setCursor(58, 15);
  tft.setTextSize(3);
  tft.println("SNIFER"); // set label for project name
}

// a function that sets the value in the top bar  
void setActivity(String isDay, String date, String time) {
  
  // position of the "mode" label
  tft.setCursor(240, 15);
  tft.setTextSize(1);
  tft.println("Mode:");

  // position of the "mode" value (light or dark) label
  tft.setCursor(235, 30);
  tft.setTextSize(1);

  if (isDay == " " || isDay == "0") {
      tft.println("(Dark)");
       // display mode bitmap (moon or sun)
      tft.pushImage(280,3,45,45,moon_icon);  // display moon bitmap
  } 
  else {
    tft.println("(Light)");
  }

  // position of the date
  tft.setCursor(355,8);
  tft.setTextSize(2);
  if (date == "") {
    tft.println("--/--/----");
  }
  else {
     tft.println(date);
  }

  // position of the time
  tft.setCursor(355,28);
  tft.setTextSize(2);
  if (time == "") {
    tft.println("88.88.88");
  } 
  else {
    tft.println(time);
  }
  tft.setCursor(455,25);
  tft.setTextSize(1);
  tft.println("PM");
}

// function that displays displays the temperature
void setTemperature(String temp) {
  uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21); // convert color hex code to RGB
  tft.setTextColor(textColor);
  tft.setCursor(210,65);
  tft.setTextSize(2);
  tft.println("Temp");

  tft.fillTriangle(321, 53, 336, 68, 336, 53, textColor);

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(210,100);
  tft.setTextSize(7);

  if (temp = "") {
    tft.println("--");
  }
  else {
     tft.println(temp);
  }


  tft.fillRect(210, 160, 120, 15, textColor);

  tft.drawCircle(305, 113, 3, textColor);
  tft.drawCircle(305, 113, 4, textColor);
  tft.setTextColor(textColor);
  tft.setCursor(310,115);
  tft.setTextSize(3);
  tft.println("C");
  tft.setTextColor(TFT_WHITE);
}

// function that displays the humidity
void setHumidity(String humidity) {
  uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21); // convert color hex code to RGB
  tft.setTextColor(textColor);
  tft.setCursor(350,65);
  tft.setTextSize(2);
  tft.println("Humidity");

  tft.fillTriangle(460, 53, 475, 68, 475, 53, textColor);

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(350,100);
  tft.setTextSize(7);

  if (humidity = "") {
      tft.println("--");
  }
  else {
    tft.println(humidity);
  }

  tft.fillRect(350, 160, 120, 15, textColor);

  tft.setTextColor(textColor);
  tft.setCursor(445,115);
  tft.setTextSize(3);
  tft.println("%");
  tft.setTextColor(TFT_WHITE);
}


void setStatus(int reading) {
  // position of the "mode" label
  tft.setCursor(10, 287);
  tft.setTextSize(2);
  tft.println("Status:");

  byte state;
  String msg_state;

  if (reading < 400) {
    state = 0;
  }
  else if (reading >= 400 && reading < 700) {
    state = 1;
  } 
  else if (reading >= 700 && reading < 900) {
    state = 2;
  }
  else if (reading >= 900) {
    state = 3;
  }

  switch(state) {
    case 0: tft.pushImage(95,271,45,45,tick_icon); msg_state="(GOOD)"; break;       // display green tick
    case 1: tft.pushImage(95,271,45,45,warning_icon); msg_state="(FAIR)"; break;    // display red cross
    case 2: tft.pushImage(95,271,45,45,cross_icon); msg_state="(POOR)"; break;      // display warning sign
    case 3: tft.pushImage(95,271,45,45,danger_icon); msg_state="(DANGER)"; break;   // display skull and bones
    default: tft.pushImage(95,271,45,45,tick_icon); msg_state="(GOOD)"; break;       // display green tick
  }

  // position of the "mode" value (light or dark) label
  tft.setCursor(145, 290);
  tft.setTextSize(1);
  tft.println(msg_state);
}



// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, const char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option
  
  x += r; y += r;   // Calculate coords of centre of ring

  int w = r / 3;    // Width of outer ring is 1/4 of radius
  
  int angle = 150;  // Half the sweep angle of meter (300 degrees)

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 3; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Variable to save "value" text colour from scheme and set default
  int colour = TFT_GREEN;
 
  // Draw colour blocks every inc degrees
  for (int i = -angle+inc/2; i < angle-inc/2; i += inc) {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v) { // Fill in coloured segments with 2 triangles
      switch (scheme) {
        case 0: colour = TFT_RED; break; // Fixed colour
        case 1: colour = TFT_GREEN; break; // Fixed colour
        case 2: colour = TFT_BLUE; break; // Fixed colour
        case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
        case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break; // Green to red (high temperature etc)
        case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
        default: colour = TFT_BLUE; break; // Fixed colour
      }
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      //text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREY);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 3; if (value > 999) len = 5;
  dtostrf(value, len, 0, buf);
  buf[len] = ' '; buf[len+1] = 0; // Add blanking space and terminator, helps to centre text too!
  // Set the text colour to default
  tft.setTextSize(2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  tft.setTextColor(colour, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  // Print value, if the meter is large then use big font 8, othewise use 4
  if (r > 84) {
    tft.setTextPadding(55*3); // Allow for 3 digits each 55 pixels wide
    tft.drawString(buf, x, y, 8); // Value in middle
  }
  else {
    tft.setTextPadding(3 * 14); // Allow for 3 digits each 14 pixels wide
    tft.drawString(buf, x+1, y, 4); // Value in middle


  }
  tft.setTextSize(1);
  tft.setTextPadding(0);
  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (r > 84) tft.drawString(units, x, y + 60, 4); // Units display
  else {
    //tft.drawString(units, x, y + 35, 2); // Units display
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(x-15,y+27);
    tft.println("PPM");
  }

  // label
  tft.setTextSize(2);
  tft.setTextColor(textColor, TFT_BLACK);
  tft.setCursor(x-65,y+85);
  tft.println("Air Quality");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // triangle
  //tft.fillTriangle(181, 53, 196, 68, 196, 53, textColor);


  // Calculate and return right hand side x coordinate
  return x + r;
}



// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

int split(String input, String output[], char delimiter) {
  int tokenIndex = 0;
  int startIndex = 0;
  
  for (int i = 0; i < input.length(); i++) {
    if (input.charAt(i) == delimiter) {
      output[tokenIndex] = input.substring(startIndex, i);
      startIndex = i + 1;
      tokenIndex++;
    }
  }
  
  // Add the last token after the last comma
  output[tokenIndex] = input.substring(startIndex);
  tokenIndex++;
  
  return tokenIndex;
}



