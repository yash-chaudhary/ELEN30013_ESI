/*
 Adapted from the Adafruit graphicstest sketch, see original header at end
 of sketch.

 This sketch uses the GLCD font (font 1) only.

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
*/

// ILI9488 dimensions: 320 x 480

#include "SPI.h"
#include "TFT_eSPI.h"
#include "nose_low_res.h"

TFT_eSPI tft = TFT_eSPI();

unsigned long total = 0;
unsigned long tn = 0;


void setup() {
  
  Serial.begin(9600);
  while (!Serial);

  tft.init();                     // initialise TFT LCD display

  uint16_t bgColor = 0x18c3;
  // uint16_t bgColor = tft.color565(0x1a, 0x1a, 0x1a); // convert color hex code to RGB

  tft.fillScreen(bgColor);        // set default background color to black
  tft.setRotation(1);             // set LCD to landscape orientation
  tft.setTextSize(2);             // Set the text size
  tft.setTextColor(TFT_WHITE);    // Set the text color
  tft.setSwapBytes(true);         // setup needed if using images
}

void loop(void) {
  
    //testText();
    //delay(2000);
    drawContainers();
    setName();
    setActivity();
    setTemperature();
    setHumidity();

    delay(5000);
}

// function that draws container boundaries
void drawContainers() {
  //uint16_t lineColor = tft.color565(0x9e, 0x9e, 0x9e); // convert color hex code to RGB

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

  // temp sensor, humidity lvl and buzzer freq output divider
  tft.drawFastHLine(200, 185, 479, lineColor);  
  tft.drawFastHLine(200, 184, 479, lineColor);  

  // temp sensor and humidity container divider (thickness of 2)
  tft.drawFastVLine(340, 49, 135, lineColor);  
  tft.drawFastVLine(339, 49, 135, lineColor);  

  // temp and humidity bg color
  uint16_t tempHumBg = tft.color565(0x44, 0x44, 0x44); // convert color hex code to RGB

  tft.fillRect(201, 51, 138, 133, tempHumBg);

  // humidity container bg styles
  tft.fillRect(341, 51, 136, 133, tempHumBg);
}

// a function that sets the name and bitmap for the project
void setName() {
  tft.pushImage(2,2,48,48,nose_low_res);  // display nose bit map
  tft.setCursor(58, 15);
  tft.setTextSize(3);
  tft.println("SNIFER"); // set label for project name
}

// a function that sets the value in the top bar  
void setActivity() {
  
  // position of the "status" label
  tft.setCursor(240, 5);
  tft.setTextSize(1);
  tft.println("Status");

  // position of the "mode" label
  tft.setCursor(300,5);
  tft.setTextSize(1);
  tft.println("Mode");

  // position of the date
  tft.setCursor(355,8);
  tft.setTextSize(2);
  tft.println("08/10/2023");

  // position of the time
  tft.setCursor(355,28);
  tft.setTextSize(2);
  tft.println("19.03.59");
  tft.setCursor(455,25);
  tft.setTextSize(1);
  tft.println("PM");
}

// function that displays displays the temperature
void setTemperature() {
  uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21); // convert color hex code to RGB
  tft.setTextColor(textColor);
  tft.setCursor(210,65);
  tft.setTextSize(2);
  tft.println("Temp");

  tft.fillTriangle(321, 53, 336, 68, 336, 53, textColor);

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(210,100);
  tft.setTextSize(7);
  tft.println("27");

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
void setHumidity() {
  uint16_t textColor = tft.color565(0xeb, 0x8e, 0x21); // convert color hex code to RGB
  tft.setTextColor(textColor);
  tft.setCursor(350,65);
  tft.setTextSize(2);
  tft.println("Humidity");

  tft.fillTriangle(460, 53, 475, 68, 475, 53, textColor);

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(350,100);
  tft.setTextSize(7);
  tft.println("65");

  tft.fillRect(350, 160, 120, 15, textColor);

  tft.setTextColor(textColor);
  tft.setCursor(445,115);
  tft.setTextSize(3);
  tft.println("%");
  tft.setTextColor(TFT_WHITE);
}

// a function that displays the air quality






