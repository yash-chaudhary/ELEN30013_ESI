// necessary libraries
#include <stdbool.h>
#include <math.h>
#include <dht.h>            // include the library http://arduino.cc/playground/Main/DHTLib
#include "MQ135.h"
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <SoftwareSerial.h> // default software serial library

// defines
#define DHT11_PIN 3

// software serial
const int rxPin = 7;
const int txPin = 8;
SoftwareSerial mySerial (rxPin, txPin);

// MQ135 Sensor
int co2_ppm;
int sensorPinAnalogue = A1;
MQ135 gasSensor(sensorPinAnalogue, 200);

// RTC timer
int clk_pin = 11;
int dat_pin = 12;
int rst_pin = 13;
ThreeWire myWire(dat_pin,clk_pin,rst_pin); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// thermistor variables
dht DHT;
float curr_humidity;
int thermistorPin = A4;
float curr_temp;
float Vo;
float Rt;
float T;
float Ro = 10000.0;
float To = 25.0;
float B = 2200.0;

// LDR variables
const int ldrPin = A0;
const float ldrThreshold = 200.0;
bool is_day;

// LED & Buzzer variables
const int redPin = A3;
const int greenPin = A2;
const int buzzPin = 4;

// Switch variables
const int switchPin1 = 5;
const int switchPin2 = 6;
int state1;
int state2;
unsigned long currentMillis = 0;
unsigned long previousBuzzMillis = 0;
unsigned long previousPacketMillis = 0;

// function prototypes
int get_co2_ppm();
RtcDateTime get_time();
float get_temp();
float fet_humidity();
bool get_light();
void buzz_LED(float co2_ppm);


// setup
void setup() {
  Serial.begin(9600);   // 9600 bits per second baud rate
  mySerial.begin(9600); // start software serial

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(ldrPin, INPUT);

  pinMode(switchPin1, INPUT_PULLUP);
  pinMode(switchPin2, INPUT_PULLUP);

  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(compiled);
}


// main loop
void loop() {
  currentMillis = millis();

  state1 = digitalRead(switchPin1);
  state2 = digitalRead(switchPin2);

  if (state1 == HIGH && state2 == HIGH) // Demo mode
  {
    run_demo();
  }
  else if (state1 == LOW) // Normal operation
  {
    RtcDateTime curr_time = get_time();
    co2_ppm = get_co2_ppm();
    curr_temp = get_temp();            // get current temperature (deg C)
    curr_humidity = get_humidity();    // get current humidity (%)
    is_day = get_light();              // get light or day (bool)
    co2_ppm = analogRead(ldrPin);
    buzz_LED(co2_ppm);			           // update buzzer and led

    if (currentMillis - previousPacketMillis >= 1000) {
      String packet = String(demo_ppm) + "," + String(demo_temp) + "," + String(demo_humidity) + "," + String(demo_is_day);
      mySerial.println(packet);
      previousPacketMillis = millis();
    }

  }
  else if (state2 == LOW) // Off mode
  {
    setColour(0, 0);
    Serial.println("Switch is in the OFF position");
    //delay(500);
  }
  
}

void run_demo() {

  int demo_ppm = 0;
  float demo_temp = 0;
  int demo_humidity = 0;
  bool demo_is_day = true;
  
  while (state1 == HIGH && state2 == HIGH) {
    buzz_LED(demo_ppm);	  // update buzzer and led

    currentMillis = millis();
    if (currentMillis - previousPacketMillis >= 1000) {
  
      if (demo_ppm == 1000) {
        demo_ppm = 0;
        demo_temp = 0;
        demo_humidity = 0;
        demo_is_day = !demo_is_day;
      }

      demo_ppm += 10;
      demo_temp += 0.5;
      demo_humidity += 1;

      RtcDateTime now = Rtc.GetDateTime();

      Serial.print("time delta: ");
      Serial.println(currentMillis - previousPacketMillis);

      Serial.print("demo_is_day: ");
      Serial.println(demo_is_day);
      Serial.print("demo_ppm: ");
      Serial.println(demo_ppm);
      Serial.print("Temperature reading (C): ");
      Serial.println(demo_temp);
      Serial.print("Humidity reading (%): ");
      Serial.println(demo_humidity);
      printDateTime(now);

      // Convert demo variables to ASCII characters and create a packet
      String packet = String(demo_ppm) + "," + String(demo_temp) + "," + String(demo_humidity) + "," + String(demo_is_day);

      // Send the packet through mySerial
      Serial.print("packet: ");
      Serial.println(packet);

      previousPacketMillis = millis();
    }

    state1 = digitalRead(switchPin1);
    state2 = digitalRead(switchPin2);
  }
}

// returns: MQ135 CO2 concentration in ppm
int get_co2_ppm() {
  co2_ppm = gasSensor.getPPM(); // Read analog input pin A5
  
  if (co2_ppm < 5) {
    co2_ppm = 5;
  } else if (co2_ppm > 1000) {
    co2_ppm = 1000;
  }
  
  Serial.print("co2_ppm: ");
  Serial.println(co2_ppm);
  //delay(1000);

  return co2_ppm;
}

RtcDateTime get_time() {
  RtcDateTime now = Rtc.GetDateTime();

  printDateTime(now);
  Serial.println();
}

// returns: current temperature in degrees celcius
float get_temp() {
  Vo = analogRead(thermistorPin);
  Vo = Vo * 5 / 1024;
  
  // Serial.print("Voltage reading (V): ");
  // Serial.println(Vo);

  Rt = Vo * Ro / (5 - Vo);

  // Serial.print("Resistance reading (Ohms): ");
  // Serial.println(Rt);

  int chk = DHT.read11(DHT11_PIN);
  float T_DHT = DHT.temperature;

  // For DHT11 calibration
  // B = 1.0 * log(Rt / Ro) / (1 / (273 + T_DHT) - 1 / (273 + To));
  // Serial.print("B reading: ");
  // Serial.println(B);

  T = 1.0 / (1.0 / B * log(Rt / Ro) + 1 / (273 + To)) - 273;

  Serial.print("Temperature reading (C): ");
  Serial.println(T);
  Serial.println();

  return T;
}

// returns: current temperature as a % of water vapor in air
float get_humidity() {

  int chk = DHT.read11(DHT11_PIN);
  float H_DHT = DHT.humidity;
  Serial.print("Humidity reading (%): ");
  Serial.println(H_DHT);

  return H_DHT;
}

// returns: false if night detected and true is day detected
bool get_light() {
  int ldrReading = analogRead(ldrPin);
  
  if (ldrReading <= ldrThreshold) {
    Serial.print("It's NIGHT: ");
    Serial.println(ldrReading);
    return false;
  } else {
    Serial.print("It's DAY: ");
    Serial.println(ldrReading);
    return true;
  }
}

void buzz_LED(float co2_ppm) {
  // Linear map of CO2 concentration in the air
  // LED green blinking for good air, then to orange for mid air, red for poor air
  // Buzzer will only buzz if air is mid or worse, buzz faster and higher pitch for poorer air

  if (co2_ppm < 400) {
    // Green
    setColour(0, 255);

  } else if (co2_ppm >= 400 && co2_ppm <= 700) {
    // Orange
    setColour(255, 165);

  } else if (co2_ppm > 700) {
    // Red
    setColour(255, 0);
  }

  int toneFreq = map(co2_ppm, 5, 1000, 31, 1800);
  // Serial.print("Buzz Freq: ");
  // Serial.println(toneFreq);

  int toneDuration = map(co2_ppm, 5, 1000, 1000, 10);
  // Serial.print("Buzz Duration: ");
  // Serial.println(toneDuration);

  if (currentMillis - previousBuzzMillis >= toneDuration) {
    tone(buzzPin, toneFreq, 100);
    previousBuzzMillis = millis();
  }
}

void setColour (int redValue, int greenValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
}

void printDateTime(const RtcDateTime& dt)
{
    char datestring[11];
    char timestring[9];

    snprintf_P(datestring, 
            11,
            PSTR("%02u/%02u/%04u"),
            dt.Month(),
            dt.Day(),
            dt.Year() );

    snprintf_P(timestring, 
            9,
            PSTR("%02u:%02u:%02u"),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );

    Serial.println(datestring);
    Serial.println(timestring);
}