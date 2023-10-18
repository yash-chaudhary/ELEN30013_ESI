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
int air_ppm;
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
int curr_humidity;
int thermistorPin = A4;
int curr_temp;
float Vo;
float Rt;
float T;
float Ro = 10000.0;
float To = 25.0;
float B = 2200.0;

// LDR variables
const int ldrPin = A0;
const float ldrThreshold = 400.0;
bool is_day;

// LED & Buzzer variables
const int redPin = A3;
const int greenPin = A2;
const int buzzPin = 4;
int ledState = LOW;

// Switch variables
const int switchPin1 = 5;
const int switchPin2 = 6;
int state1;
int state2;
unsigned long currentMillis = 0;
unsigned long previousBuzzMillis = 0;
unsigned long previousBlinkMillis = 0;
unsigned long previousPacketMillis = 0;

// function prototypes
void run_normal();
void run_demo();
void printSerial(int air_ppm, int curr_temp, int curr_humidity, bool is_day);
int get_air_ppm();
int get_temp();
int get_humidity();
bool get_light();
void buzz_LED(int air_ppm);
void setColour (int redValue, int greenValue);

// setup
void setup() {
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.SetDateTime(compiled);

  Serial.begin(9600);   // 9600 bits per second baud rate
  mySerial.begin(9600); // start software serial

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(ldrPin, INPUT);

  pinMode(switchPin1, INPUT_PULLUP);
  pinMode(switchPin2, INPUT_PULLUP);

  for (int freq = 500; freq <= 1000; freq += 10) {
    tone(buzzPin, freq, 100);
    delay(5);
  }
  delay(1000);
}


// main loop
void loop() {

  state1 = digitalRead(switchPin1);
  state2 = digitalRead(switchPin2);

  if (state1 == HIGH && state2 == HIGH) // Demo mode
  {
    run_demo();
  }
  else if (state1 == LOW) // Normal operation
  {
    run_normal();
  }
  else if (state2 == LOW) // Off mode
  {
    setColour(0, 0);
    Serial.println("Switch is in the OFF position");
    delay(500);
  }
  
}

void run_normal() {

  tone(buzzPin, 1000, 100);
  delay(200);
  tone(buzzPin, 1000, 100);
  delay(1000);

  while (state1 == LOW) {
    buzz_LED(air_ppm);	  // update buzzer and led

    currentMillis = millis();
    if (currentMillis - previousPacketMillis >= 500) {
      Serial.print("time delta: ");
      Serial.println(currentMillis - previousPacketMillis);

      RtcDateTime curr_time = Rtc.GetDateTime();
      air_ppm = get_air_ppm();
      curr_temp = get_temp();            // get current temperature (deg C)
      curr_humidity = get_humidity();    // get current humidity (%)
      is_day = get_light();              // get light or day (bool)

      air_ppm = analogRead(ldrPin);
  
      printSerial(air_ppm, curr_temp, curr_humidity, is_day);

      previousPacketMillis = millis();
    }

    state1 = digitalRead(switchPin1);
    state2 = digitalRead(switchPin2);
  }
}

void run_demo() {

  tone(buzzPin, 500, 100);
  delay(200);
  tone(buzzPin, 1000, 100);
  delay(200);
  tone(buzzPin, 1500, 100);
  delay(1000);

  int demo_ppm = 0;
  int demo_temp = 0;
  int demo_humidity = 0;
  bool demo_is_day = true;
  
  while (state1 == HIGH && state2 == HIGH) {
    buzz_LED(demo_ppm);	  // update buzzer and led

    currentMillis = millis();
    if (currentMillis - previousPacketMillis >= 500) {
      Serial.print("time delta: ");
      Serial.println(currentMillis - previousPacketMillis);
  
      if (demo_ppm == 1000) {
        demo_ppm = 0;
        demo_temp = 0;
        demo_humidity = 0;
        demo_is_day = !demo_is_day;
      }

      demo_ppm += 20;
      demo_temp += 1;
      demo_humidity += 2;

      printSerial(demo_ppm, demo_temp, demo_humidity, demo_is_day);

      previousPacketMillis = millis();
    }

    state1 = digitalRead(switchPin1);
    state2 = digitalRead(switchPin2);
  }
}

void printSerial(int air_ppm, int curr_temp, int curr_humidity, bool is_day) {

  RtcDateTime curr_time = Rtc.GetDateTime();

  Serial.print("air_ppm: ");
  Serial.println(air_ppm);
  Serial.print("Temperature reading (C): ");
  Serial.println(curr_temp);
  Serial.print("Humidity reading (%): ");
  Serial.println(curr_humidity);
  Serial.print("is_day: ");
  Serial.println(is_day);

  char datestring[11];
  char timestring[9];

  snprintf_P(datestring, 
          11,
          PSTR("%02u/%02u/%04u"),
          curr_time.Day(),
          curr_time.Month(),
          curr_time.Year() );

  snprintf_P(timestring, 
          9,
          PSTR("%02u:%02u:%02u"),
          curr_time.Hour(),
          curr_time.Minute(),
          curr_time.Second() );

  // Convert demo variables to ASCII characters and create a packet
  String packet = String(air_ppm) + "," + String(curr_temp) + "," + String(curr_humidity) + "," + String(is_day) + "," + String(datestring) + "," + String(timestring);

  // Send the packet through mySerial
  mySerial.println(packet);

  Serial.print("packet: ");
  Serial.println(packet);
}

// returns: MQ135 sensor concentration in ppm
int get_air_ppm() {
  air_ppm = gasSensor.getPPM(); // Read analog input pin A5
  
  if (air_ppm < 5) {
    air_ppm = 5;
  } else if (air_ppm > 1000) {
    air_ppm = 1000;
  }

  return air_ppm;
}

// returns: current temperature in degrees celcius
int get_temp() {
  Vo = analogRead(thermistorPin);
  Vo = Vo * 5 / 1024;
  Rt = Vo * Ro / (5 - Vo);

  T = 1.0 / (1.0 / B * log(Rt / Ro) + 1 / (273 + To)) - 273;

  return T;
}

// returns: current temperature as a % of water vapor in air
int get_humidity() {
  int chk = DHT.read11(DHT11_PIN);
  int H_DHT = DHT.humidity;

  return H_DHT;
}

// returns: false if night detected and true is day detected
bool get_light() {
  int ldrReading = analogRead(ldrPin);
  
  if (ldrReading <= ldrThreshold) {
    //Serial.print("It's NIGHT: ");
    return false;
  } else {
    // Serial.print("It's DAY: ");
    return true;
  }
}

void buzz_LED(int air_ppm) {
  // Linear map of sensor concentration in the air
  // LED green blinking for good air, then to orange for mid air, red for poor air
  // Buzzer will, buzz faster and higher pitch for poorer air

  int blinkDuration = map(air_ppm, 0, 1000, 1000, 100);

  if (currentMillis - previousBlinkMillis >= blinkDuration) {
    if (ledState == LOW) {  // if the LED is off turn it on and vice-versa
      ledState = HIGH;   //change led state for next iteration
      setColour(0, 0);

    } else {
      ledState = LOW;

      if (air_ppm < 400) {
        // Green
        setColour(0, 255);

      } else if (air_ppm >= 400 && air_ppm <= 700) {
        // Orange
        setColour(255, 165);

      } else if (air_ppm > 700) {
        // Red
        setColour(255, 0);
      }
    }

    previousBlinkMillis = millis();
  }

  int toneFreq = map(air_ppm, 0, 1000, 31, 1800);
  // Serial.print("Buzz Freq: ");
  // Serial.println(toneFreq);

  int toneDuration = map(air_ppm, 0, 1000, 1000, 100);
  // Serial.print("Buzz Duration: ");
  // Serial.println(toneDuration);

  if (currentMillis - previousBuzzMillis >= toneDuration && air_ppm > 200) {
    tone(buzzPin, toneFreq, 100);
    previousBuzzMillis = millis();
  }
}

void setColour (int redValue, int greenValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
}