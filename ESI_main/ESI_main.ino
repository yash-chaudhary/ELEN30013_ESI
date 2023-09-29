// necessary libraries
#include <stdbool.h>
#include <math.h>
#include <dht.h>            // include the library http://arduino.cc/playground/Main/DHTLib
#include <LiquidCrystal.h>  // include the library https://www.arduino.cc/en/Reference/LiquidCrystal

// defines
#define DHT11_PIN 3

// thermistor variables
int thermistorPin = 1;
float Vo;
float Rt;
float T;
float Ro = 10000.0;
float To = 25.0;
float B = 2200.0;

// LDR variables
const int ldrPin = A1;
const float ldrThreshold = 200.0;

// LED & Buzzer variables
const int redPin = 7;
const int greenPin = 6;
const int bluePin = 5;
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
  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);
}


// main loop
void loop() {
  float curr_temp = get_temp();           // get current temperature (deg C)
  float curr_humidity = get_humidity();   // get current humidity (%)
  bool is_day = get_light();              // get light or day (bool)
  buzz_LED(co2_ppm);			  // update buzzer and led
}

// returns: current temperature in degrees celcius
float get_temp() {
  Vo = analogRead(thermistorPin);
  Vo = Vo * 5 / 1024;
  
  Serial.print("Voltage reading (V): ");
  Serial.println(Vo);

  Rt = Vo * Ro / (5 - Vo);

  Serial.print("Resistance reading (Ohms): ");
  Serial.println(Rt);

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

  return H_DHT;
}

// returns: false if night detected and true is day detected
bool get_light() {
  int ldrReading = analogRead(ldrPin);
  
  if (ldrReading <= ldrThreshold) {
    Serial.print("It's NIGHT: ");
    Serial.println(ldrReading);
    return False;
  } else {
    digitalWrite(ledPin, LOW);
    Serial.print("It's DAY: ");
    Serial.println(ldrReading);
    return True;
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

  int toneFreq = map(co2_ppm, 10, 1000, 31, 15000);
  tone(buzzPin, toneFreq);
}

void setColour (int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}