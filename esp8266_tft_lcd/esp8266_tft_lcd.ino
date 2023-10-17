
/*
  Author: Yash Chaudhary
  Date: 16/10/2023
  NOTES: Ensure TFT LCD setup files have been properly setup

  Some dodgy shortcuts taken:
  i) added whitespace to smaller words to ensure longer words aren't visible after LCD re-write
*/

// ILI9488 dimensions: 320 x 480

// include libraries and files
#include "SPI.h"
#include "TFT_eSPI.h"
#include "light_nose_icon.h"      // bitmap 48 x 48 px 
#include "dark_nose_icon.h"      // bitmap 48 x 48 px 
#include "moon_icon.h"      // bitmap 45 x 45 px
#include "sun_icon.h"      // bitmap 45 x 45 px
#include "dark_tick_icon.h"      // bitmap 45 x 45 px
#include "dark_warning_icon.h"   // bitmap 45 x 45 px
#include "dark_cross_icon.h"     // bitmap 45 x 45 px
#include "dark_danger_icon.h"    // bitmap 45 x 45 px
#include "light_tick_icon.h"      // bitmap 45 x 45 px
#include "light_warning_icon.h"   // bitmap 45 x 45 px
#include "light_cross_icon.h"     // bitmap 45 x 45 px
#include "light_danger_icon.h"    // bitmap 45 x 45 px
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

// custom colors
#define TFT_GREY 0x528a
uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21);
bool prevLightMode = false;
bool lightMode = false;

// graph configurations
GraphWidget gr = GraphWidget(&tft); 
TraceWidget tr1 = TraceWidget(&gr); // traces are drawn on tft using graph instance
TraceWidget tr2 = TraceWidget(&gr); // traces are drawn on tft using graph instance
float static gx = 0.0;

// control execution
static unsigned long interval;


//####################################################################
/*                          Setup Function                          */
//####################################################################
void setup() {

    // start serial monitor
    Serial.begin(baudrate);
    while (!Serial);

    // start software serial
    mySerial.begin(baudrate);
    while (!mySerial);
  
    // tft LCD setup
    tft.init();                                 // initialise TFT LCD display
    tft.fillScreen(TFT_BLACK);                  // set default background color to black
    tft.setRotation(1);                         // set LCD to landscape orientation
    tft.setTextColor(TFT_WHITE, TFT_BLACK);     // global text color
    tft.setSwapBytes(true);                     // allow use of images (bitmaps)

    // graph initialisations
    gr.createGraph(240, 75, TFT_BLACK);            // graph area is 240 pixels wide, 75 high, TFT_BLACK background
    gr.setGraphScale(0.0, 5000.0, 0.0, 1800.0);    // x scale units is from 0 to 100, y scale units is -50 to 50

    // X grid starts at 0 with lines every 10 x-scale units
    // Y grid starts at -50 with lines every 25 y-scale units
    gr.setGraphGrid(0.0, 500.0, 0, 600, TFT_WHITE);

    gr.drawGraph(230, 235);         // draw empty graph at xpos = 230 and ypos = 235
    tr1.startTrace(textColor);      // start a trace with line color as textColor 
    tr1.addPoint(0.0, 0.0);         // add points on graph to trace 1 using graph scale factors
    tr2.startTrace(textColor);      // start second trace (to increase line thickness)
    tr2.addPoint(0.0+1.0, 0.0+1.0); // add points on graph to trace 1 using graph scale factors

    // Draw the x axis scale
    // tft.setTextDatum(TC_DATUM); // Top centre text datum
    // tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(-50.0) + 3);
    // tft.drawNumber(50, gr.getPointX(50.0), gr.getPointY(-50.0) + 3);
    // tft.drawNumber(100, gr.getPointX(100.0), gr.getPointY(-50.0) + 3);

    // Draw the y axis scale
    tft.setTextDatum(MR_DATUM); // Middle right text datum
    tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(0.0));
    tft.drawNumber(600, gr.getPointX(0.0), gr.getPointY(600.0));
    tft.drawNumber(1200, gr.getPointX(0.0), gr.getPointY(1200.0));
    tft.drawNumber(1800, gr.getPointX(0.0), gr.getPointY(1800.0));
}


//####################################################################
/*                         Superloop Function                       */
//####################################################################
void loop(void) {

    if (millis() - interval >= 500) {
    
        // check data in serial buffer
        if(mySerial.available() > 0) {

            incomingString = mySerial.readStringUntil('\n');

            // split the input string into individual values based on commas
            int numValues = split(incomingString, values, ',');
        }

        // pull out return values
        String ppm = values[0];
        String temp = values[1];
        String humidity = values[2];
        String isDay = values[3];
        String date = values[4];
        String time = values[5];

        // set lightMode state globally
        if (isDay == "0") {
            lightMode = false;
        } else if (isDay == "1") {
            lightMode = true;
        }

        // make LCD fill screen only if new lightMode different from previousLightMode
        // graph is drawn again when mode is changed ensuring styles are updated
        if (lightMode != prevLightMode) {
            prevLightMode = lightMode;
            if (lightMode == false) {
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                gr.createGraph(240, 75, TFT_BLACK);
                gr.setGraphGrid(0.0, 500.0, 0, 600, TFT_WHITE);
                gr.setGraphScale(0.0, 5000.0, 0.0, 1800.0);    // x scale units is from 0 to 100, y scale units is -50 to 50
                gr.drawGraph(230, 235);     // draw empty graph at 230, 235 on display
                tr1.startTrace(textColor);
                tr2.startTrace(textColor);
                tft.setTextDatum(MR_DATUM); // Middle right text datum
                tft.setTextSize(1);
                tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(0.0));
                tft.drawNumber(600, gr.getPointX(0.0), gr.getPointY(600.0));
                tft.drawNumber(1200, gr.getPointX(0.0), gr.getPointY(1200.0));
                tft.drawNumber(1800, gr.getPointX(0.0), gr.getPointY(1800.0));
                gx = 0;
                
            } else if (lightMode == true) {
                tft.fillScreen(TFT_WHITE);
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
                gr.createGraph(240, 75, TFT_WHITE);
                gr.setGraphGrid(0.0, 500.0, 0, 600, TFT_BLACK);
                gr.setGraphScale(0.0, 5000.0, 0.0, 1800.0);    // x scale units is from 0 to 100, y scale units is -50 to 50
                gr.drawGraph(230, 235);     // draw empty graph at 230, 235 on display
                tr1.startTrace(textColor);
                tr2.startTrace(textColor);
                tft.setTextDatum(MR_DATUM); // Middle right text datum
                tft.setTextSize(1);
                tft.drawNumber(0, gr.getPointX(0.0), gr.getPointY(0.0));
                tft.drawNumber(600, gr.getPointX(0.0), gr.getPointY(600.0));
                tft.drawNumber(1200, gr.getPointX(0.0), gr.getPointY(1200.0));
                tft.drawNumber(1800, gr.getPointX(0.0), gr.getPointY(1800.0));
                gx = 0;
            }
        }

        // NOTE: need to draw new graph when color changes as grid wont change otherwise

        drawContainers();                   // draw bounding containers
        setName();                          // set top bar name and bitmap
        setActivity(isDay, date, time);     // set mode, date, time
        setTemperature(temp);               // set temp
        setHumidity(humidity);              // set humidity

        // convert ppm char value to into
        if (ppm == "") {
          reading = 0;
        } else {
          reading = ppm.toInt();
        }

        // draw air quality meter
        ringMeter(reading,0,999, xpos, ypos, radius,"PPM",GREEN2RED); // Draw analogue meter

        // set air quality status
        setStatus(reading);

        // initialise graph point paramters  
        long gy_tmp, delta;

        // map ppm value to know frequency range (31 Hz to 1800 Hz)
        long gy = map(values[0].toInt(), 0, 1000, 31, 1800);
        
        // setup text label for frequency graph container
        lightMode ? tft.setTextColor(textColor, TFT_WHITE) : tft.setTextColor(textColor, TFT_BLACK);
        tft.setCursor(210, 200);
        tft.setTextSize(2);
        tft.println("Buzzer Freq (Hz):");
        lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(420, 200);
        tft.setTextSize(2);

        // specify additional whitespace based digits in frequency value
        if (gy > 999 ) {
            tft.println(String(gy));
        } else if (gy > 99) {
            tft.println(String(gy) + " ");
        } else {
            tft.println(String(gy) + "  ");
        }

        // calculate delta in y-value 
        if (gx == 0.0) {
            delta = 0.0;
            gy_tmp = gy;
        } else {
            if (gy > gy_tmp) {
                delta = gy - gy_tmp;
            } else {
                delta = gy_tmp - gy;
            }
        }

        // Add a new point on each trace
        tr1.addPoint(gx, gy);
        tr2.addPoint(gx+1.0, gy+1.0);   // Add points on graph to trace 1 using graph scale factors

        // Create next plot point   
        gx += 500;
        gy += delta;  

        // if the end of the graph is reached start 2 new traces
        // note that each new gridpoint in x-axis is 500 
        if (gx > 5500) {
          gx = 0.0;
          
          gr.drawGraph(230, 235);     // draw empty graph at 230, 235 on display
          tr1.startTrace(textColor);  // start new trace
          tr2.startTrace(textColor);  // start new trace 
        }

        // increase interval
        interval += 500;
    }
}


//####################################################################
// (void) function that draws container boundaries                   #
//####################################################################
void drawContainers() {

    uint16_t lineColor;

    if (lightMode) {
        lineColor = TFT_BLACK;
    }
    else {
        lineColor = TFT_WHITE;
    }


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


//####################################################################
// (void) function that sets the name and bitmap for the project     #
//####################################################################
void setName() {
    lightMode ? tft.pushImage(2,2,48,48,light_nose_icon): tft.pushImage(2,2,48,48,dark_nose_icon);  // display nose bit map
    if (lightMode) {
        tft.drawFastVLine(50, 2, 49, TFT_BLACK);
    }
    tft.setCursor(58, 15);
    tft.setTextSize(3);
    tft.println("SNIFER"); // set label for project name
}


//####################################################################
// (void) function that sets the mode, time and date in the top bar  #
//####################################################################
void setActivity(String isDay, String date, String time) {
  
    // position of the "mode" label
    tft.setCursor(235, 15);
    tft.setTextSize(1);
    tft.println("Mode:");

    // position of the "mode" value (light or dark) label
    tft.setCursor(230, 30);
    tft.setTextSize(1);

    lightMode ? tft.pushImage(280,3,45,45,sun_icon) : tft.pushImage(280,3,45,45,moon_icon);

    if (isDay == " " || isDay == "0") {
        tft.println("(Dark) ");
        // display mode bitmap (moon or sun)
    } else {
        tft.println("(Light)");
    }

    // position of the date
    tft.setCursor(355,8);
    tft.setTextSize(2);
    if (date == "") {
        tft.println("--/--/----");
    } else {
        tft.println(date);
    }

    // position of the time
    tft.setCursor(355,28);
    tft.setTextSize(2);
    if (time == "") {
        tft.println("88.88.88");
    } else {
        tft.println(time);
    }
    tft.setCursor(455,25);
    tft.setTextSize(1);

    // set AM or PM based on 24 hr clock
    int hours = time.substring(0,2).toInt();

    if (hours >= 0 && hours < 12) {
        tft.println("AM");
    } else if (hours > 12 && hours <= 23) {
        tft.println("PM");
    }
}


//####################################################################
// (void) function that displays displays the temperature            #
//####################################################################
void setTemperature(String temp) {
    uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21); // convert color hex code to RGB
    lightMode ? tft.setTextColor(textColor, TFT_WHITE) : tft.setTextColor(textColor, TFT_BLACK);
    tft.setCursor(210,65);
    tft.setTextSize(2);
    tft.println("Temp");

    tft.fillTriangle(321, 53, 336, 68, 336, 53, textColor);

    lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(210,100);
    tft.setTextSize(7);

    if (temp == "") {
    tft.println("--");
    }
    else {
        tft.println(temp);
    }

    tft.fillRect(210, 160, 120, 15, textColor);

    tft.drawCircle(305, 113, 3, textColor);
    tft.drawCircle(305, 113, 4, textColor);
    lightMode ? tft.setTextColor(textColor, TFT_WHITE) : tft.setTextColor(textColor, TFT_BLACK);
    tft.setCursor(310,115);
    tft.setTextSize(3);
    tft.println("C");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}


//####################################################################
// (void) function that displays the humidity                        #
//####################################################################
void setHumidity(String humidity) {
    uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21); // convert color hex code to RGB
    lightMode ? tft.setTextColor(textColor, TFT_WHITE) : tft.setTextColor(textColor, TFT_BLACK);

    tft.setCursor(350,65);
    tft.setTextSize(2);
    tft.println("Humidity");

    tft.fillTriangle(460, 53, 475, 68, 475, 53, textColor);

    lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);;
    tft.setCursor(350,100);
    tft.setTextSize(7);

    if (humidity == "") {
        tft.println("--");
    }
    else {
        tft.println(humidity);
    }

    tft.fillRect(350, 160, 120, 15, textColor);

    lightMode ? tft.setTextColor(textColor, TFT_WHITE) : tft.setTextColor(textColor, TFT_BLACK);
    tft.setCursor(445,115);
    tft.setTextSize(3);
    tft.println("%");
    lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);
}


//####################################################################
// (void) function that displays qualativate air quality status      #
//####################################################################
void setStatus(int reading) {

    // position of the "mode" label
    tft.setCursor(10, 287);
    tft.setTextSize(2);
    tft.println("Status:");

    // state and msg instantiation
    byte state;
    String msg_state;

    if (reading < 400) {
        state = 0;
    }   else if (reading >= 400 && reading < 700) {
        state = 1;
    } else if (reading >= 700 && reading < 900) {
        state = 2;
    }   else if (reading >= 900) {
        state = 3;
    }

    // switch icon and state label based on state
    switch(state) {
        case 0: {
            lightMode ? tft.pushImage(95,271,45,45,light_tick_icon) : tft.pushImage(95,271,45,45,dark_tick_icon);       // display green tick
            msg_state="(GOOD)  ";
            break;       
        }
        case 1: {
            lightMode ? tft.pushImage(95,271,45,45,light_warning_icon) : tft.pushImage(95,271,45,45,dark_warning_icon);  // display red cross
            msg_state="(FAIR)  ";
            break;
        }
        case 2: {
            lightMode ? tft.pushImage(95,271,45,45,light_cross_icon) : tft.pushImage(95,271,45,45,dark_cross_icon);     // display warning sign
            msg_state="(POOR)  "; 
            break;
        }
        case 3: {
            lightMode ? tft.pushImage(95,271,45,45,light_danger_icon) : tft.pushImage(95,271,45,45,dark_danger_icon);   // display skull and bones
            msg_state="(DANGER)"; 
            break; 
        } 
        default: {
            lightMode ? tft.pushImage(95,271,45,45,light_tick_icon) : tft.pushImage(95,271,45,45,dark_tick_icon);       // default display green tick
            msg_state="(GOOD)  ";
            break;  
        }      
    }

    // position of the "mode" value (light or dark) label
    tft.setCursor(145, 290);
    tft.setTextSize(1);
    lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println(msg_state);
}


//####################################################################
// (int) draw the meter, returns x coord of righthand side           #
//####################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, const char *units, byte scheme)
{
    // Minimum value of r is about 52 before value text intrudes on ring

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
        } else {   // Fill in blank segments
            tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREY);
            tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREY);
        }
    }

    // Convert value to a string
    char buf[10];
    byte len = 3; if (value > 999) len = 5; if (value < 100) len = 5;
    dtostrf(value, len, 0, buf);
    buf[len] = ' '; buf[len+1] = 0; // Add blanking space and terminator, helps to centre text too!
    // Set the text colour to default
    tft.setTextSize(2);
    // lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // Uncomment next line to set the text colour to the last segment value!
    lightMode ? tft.setTextColor(colour, TFT_WHITE) : tft.setTextColor(colour, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    // Print value, if the meter is large then use big font 8, othewise use 4
    if (r > 84) {
        tft.setTextPadding(55*3); // Allow for 3 digits each 55 pixels wide
        tft.drawString(buf, x, y, 8); // Value in middle
    } else {
        tft.setTextPadding(3 * 14); // Allow for 3 digits each 14 pixels wide
        tft.drawString(buf, x+1, y, 4); // Value in middle
    }

    tft.setTextSize(1);
    tft.setTextPadding(0);
    // Print units, if the meter is large then use big font 4, othewise use 2
    lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);
    if (r > 84) tft.drawString(units, x, y + 60, 4); // Units display
    else {
        //tft.drawString(units, x, y + 35, 2); // Units display
        tft.setTextSize(2);
        lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(x-15,y+27);
        tft.println("PPM");
    }

    // label
    tft.setTextSize(2);
    lightMode ? tft.setTextColor(textColor, TFT_WHITE) : tft.setTextColor(textColor, TFT_BLACK);
    tft.setCursor(x-65,y+85);
    tft.println("Air Quality");
    lightMode ? tft.setTextColor(TFT_BLACK, TFT_WHITE) : tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // triangle
    //tft.fillTriangle(181, 53, 196, 68, 196, 53, textColor);

    // Calculate and return right hand side x coordinate
    return x + r;
}


//####################################################################
// (int) return a 16 bit rainbow colour                              #
//####################################################################
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


//####################################################################
// (int) return indexes based on comma delimiter for incoming data   #
//####################################################################
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

