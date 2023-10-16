
/*
    Author: Yash Chaudhary
    Date: 16/10/2023
    Purpose: the purpose of this code is feed demo data to an ESP8266 in the format that it is expecting
*/

// software serial
#include <SoftwareSerial.h> // default software serial library
const int rxPin = 7;
const int txPin = 8;
SoftwareSerial mySerial (rxPin, txPin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  
  String outgoingString = createPacket();

  Serial.println(outgoingString);  // output data to serial monitor
  mySerial.println(outgoingString);      // send data to ESP8266

  delay(500);

}


// create packet in the following format
// ppm,temp,humidity,is_day,date,time
// int: ppm (0 to 999)
// int: temp (0 to 100)
// int: humidity (0 to 100)
// int: is_day (0 or 1)
// string: date (dd/mm//yyyy)
// string; time (hh:mm:ss)
String createPacket() {

  String ppm  = String(random(0,1000));
  String temp = String(random(15,51));
  String humidity = String(random(50,95));
  String is_day = String(0);
  String day = String(random(1,32));
  String month = String(random(1,13));
  String year = String(2023);
  String date = day + '/' + month + '/' + year;
  String hrs = String(random(0,25));
  String mins = String(random(0,60));
  String secs = String(random(0,60));
  String time = hrs + ':' + mins + ':' + secs;
  
  String outgoingString = ppm + ',' + temp + ',' + humidity + ',' + is_day + ',' + date + ',' + time;

  return outgoingString;
}





