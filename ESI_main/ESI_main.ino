// necessary libraries
#include <stdbool.h>
#include <math.h>
#include <dht.h>            // include the library http://arduino.cc/playground/Main/DHTLib
#include <LiquidCrystal.h>  // include the library https://www.arduino.cc/en/Reference/LiquidCrystal

// defines
#define DHT11_PIN 3

// thermistor variables
int thermistorPin = A4;
float Vo;
float Rt;
float T;
float Ro = 10000.0;
float To = 25.0;
float B = 2200.0;

// LDR variables
const int ldrPin = A0;
const float ldrThreshold = 200.0;

// LED & Buzzer variables
const int redPin = A3;
const int greenPin = A2;
const int bluePin = A1;
const int buzzPin = 4
;
// LCD variables
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;

// instantiations
dht DHT;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// function prototypes
float get_temp();
bool get_light();
void buzz_LED(float co2_ppm);


// setup
void setup() {
  Serial.begin(9600);   // 9600 bits per second baud rate
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(ldrPin, INPUT);
}


// main loop
void loop() {
  float co2_ppm = 500;
  float curr_temp = get_temp();           // get current temperature (deg C)
  float curr_humidity = get_humidity();   // get current humidity (%)
  bool is_day = get_light();              // get light or day (bool)
  co2_ppm = analogRead(ldrPin);
  buzz_LED(co2_ppm);			                // update buzzer and led

  int toneDuration = map(co2_ppm, 10, 1000, 1000, 10);
  Serial.print("Buzz Duration: ");
  Serial.println(toneDuration);

  delay(toneDuration);
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

  // CAN GET RID OF BLUE PIN SINCE IT'S ALWAYS ZEROOOOOOOOOOOO

  if (co2_ppm < 300) {
    // Green
    setColour(0, 255, 0);

  } else if (co2_ppm >= 300 && co2_ppm <= 700) {
    // Orange
    setColour(255, 165, 0);

  } else if (co2_ppm > 700) {
    // Red
    setColour(255, 0, 0);

  }

  int toneFreq = map(co2_ppm, 10, 1000, 31, 1500);
  Serial.print("Buzz Freq: ");
  Serial.println(toneFreq);

  tone(buzzPin, toneFreq, 100);
}

void setColour (int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}