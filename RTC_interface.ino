#include <math.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <LiquidCrystal.h>
// include the library https://www.arduino.cc/en/Reference/LiquidCrystal
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int clk_pin = 11;
int dat_pin = 12;
int rst_pin = 13;

ThreeWire myWire(dat_pin,clk_pin,rst_pin); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
    Serial.begin(9600);

    lcd.begin(16, 2); // set up the LCD's number of columns and rows:

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);

    if (!Rtc.IsDateTimeValid()) 
    {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    RtcDateTime now = Rtc.GetDateTime();
}

void loop() {
  RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();

    if (!now.IsValid())
    {
      Serial.println("RTC DateTime invalid");
    }

    delay(1000); // five seconds
}


#define countof(a) (sizeof(a)/sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[11];
    char timestring[9];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u"),
            dt.Month(),
            dt.Day(),
            dt.Year() );

    snprintf_P(timestring, 
            countof(timestring),
            PSTR("%02u:%02u:%02u"),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );

    //Serial.println(datestring);
    //Serial.println(timestring);

    lcd.setCursor(0,0); //set cursor to the start of the first line
    lcd.print("Date: ");
    lcd.print(datestring); // print a simple message
    lcd.setCursor(0, 1); //move cursor to the start of the second line
    lcd.print("Time: ");
    lcd.print(timestring);
}
