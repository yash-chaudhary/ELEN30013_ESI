
/*
    Author: Yash Chaudhary
    Date: 16/10/2023
    Purpose: the purpose of this code is feed demo data to an ESP8266 in the format that it is expecting
*/

char currentTime[9]; // Array to store the current time in HH:MM:SS format
char currentDate[11]; // Array to store the current date in DD/MM/YYYY format

unsigned long previousMillis = 0;  // Store the previous millis value
const unsigned long interval = 10000;  // Interval in milliseconds (30 seconds)

// software serial
#include <SoftwareSerial.h> // default software serial library
const int rxPin = 7;
const int txPin = 8;
SoftwareSerial mySerial (rxPin, txPin);
byte dayByte = 0;

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
    unsigned long currentMillis = millis();

    String ppm  = String(random(15,1000));
    String temp = String(random(15,51));
    String humidity = String(random(50,95));

    if (currentMillis - previousMillis >= interval) {
    
        previousMillis = currentMillis;

        dayByte == 0 ? dayByte = 1 : dayByte = 0;
    }

    String is_day = String(dayByte);

    generateRandomDate(currentDate);
    generateRandomTime(currentTime);

    String outgoingString = ppm + ',' + temp + ',' + humidity + ',' + is_day + ',' + currentDate + ',' + currentTime;

    return outgoingString;
}


void generateRandomTime(char* timeArray) {
    int hour = random(24);
    int minute = random(60);
    int second = random(60);
    snprintf(timeArray, 9, "%02d:%02d:%02d", hour, minute, second);
}

void generateRandomDate(char* dateArray) {
    int day = random(1, 32);
    int month = random(1, 13);
    int year = random(2023, 2030); // Adjust the range of years as needed
    snprintf(dateArray, 11, "%02d/%02d/%04d", day, month, year);
}




