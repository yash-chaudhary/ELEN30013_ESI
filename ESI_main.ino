#include <stdbool.h>
#include <dht.h>
dht DHT;
#define DHT11_PIN 3

// Thermistor variables
int ThermistorPin = 1;
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
const int ledPin = 13;
const int buzzPin = 3;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  curr_temp = get_temp();
  is_day = get_light();
  buzz_LED(co2_ppm);

  LCD_out(curr_temp, is_day, co2_ppm);
}

float get_temp() {
  Vo = analogRead(ThermistorPin);
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
}

#include <dht.h>
// include the library from LMS
// http://arduino.cc/playground/Main/DHTLib
dht DHT;
#define DHT11_PIN 3
#include <LiquidCrystal.h>
// include the library https://www.arduino.cc/en/Reference/LiquidCrystal
#include <math.h>

const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int ThermistorPin = 1; //Set value to match which analog input you are using
float Vo;
float Rt;
float T;
float Ro = 10000.0;
float To = 25.0;
float B = 2200.0;

void setup() {
  lcd.begin(16, 2); // set up the LCD's number of columns and rows:
  Serial.begin(9600);
}

void loop() {
  Vo = analogRead(ThermistorPin);
  Vo = Vo * 5 / 1024;
  
  Serial.print("Voltage reading (V): ");
  Serial.println(Vo);

  Rt = Vo * Ro / (5 - Vo);

  Serial.print("Resistance reading (Ohms): ");
  Serial.println(Rt);

  int chk = DHT.read11(DHT11_PIN);
  float T_DHT = DHT.temperature;

  //B = 1.0 * log(Rt / Ro) / (1 / (273 + T_DHT) - 1 / (273 + To));

  Serial.print("B reading: ");
  Serial.println(B);

  T = 1.0 / (1.0 / B * log(Rt / Ro) + 1 / (273 + To)) - 273;

  Serial.print("Temperature reading (C): ");
  Serial.println(T);
  Serial.println();

  lcd.setCursor(0,0); //set cursor to the start of the first line
  lcd.print("Temperature (C):"); // print a simple message
  lcd.setCursor(0, 1); //move cursor to the start of the second line
  // print the number 64 since reset:
  lcd.print(T);

  delay(1000);
}