/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-gps
 */

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

const int RXPin = 5, TXPin = 6;
const uint32_t GPSBaud = 9600; //Default baud of NEO-6M is 9600
int iterationsCounter = 0;

TinyGPSPlus gps; // the TinyGPS++ object
SoftwareSerial gpsSerial(RXPin, TXPin); // the serial interface to the GPS device

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);

  delay(500);
  Serial.println("Arduino - GPS module");
  delay(500);

}

void loop() {

  if (gpsSerial.available() > 0) {

    if (gps.encode(gpsSerial.read())) {

      Serial.print("______________________ - ");
      Serial.println(iterationsCounter);

      if (gps.location.isValid()) {
        Serial.print(F("- latitude: "));
        Serial.println(gps.location.lat(), 8);

        Serial.print(F("- longitude: "));
        Serial.println(gps.location.lng(), 8);

        Serial.print(F("- Fix Age: "));
        Serial.println(gps.location.age());

        Serial.print(F("- altitude: "));
        if (gps.altitude.isValid())
          Serial.println(gps.altitude.meters(), 4);
        else
          Serial.println(F("INVALID"));
      } else {
        Serial.println(F("- location: INVALID"));
      }

      Serial.print(F("- speed: "));
      if (gps.speed.isValid()) {
        Serial.print(gps.speed.kmph());
        Serial.println(F(" km/h"));
      } else {
        Serial.println(F("INVALID"));
      }

      Serial.print(F("- GPS date&time: "));
      if (gps.date.isValid() && gps.time.isValid()) {
        Serial.print(gps.date.year());
        Serial.print(F("-"));
        Serial.print(gps.date.month());
        Serial.print(F("-"));
        Serial.print(gps.date.day());
        Serial.print(F(" "));
        Serial.print(gps.time.hour());
        Serial.print(F(":"));
        Serial.print(gps.time.minute());
        Serial.print(F(":"));
        Serial.println(gps.time.second());
      } else {
        Serial.println(F("INVALID"));
      }

      Serial.print(F("- Satellites & Age: "));
      if (gps.satellites.isValid()) {
        Serial.print(gps.satellites.value());
        Serial.print(" - ");
        Serial.print(gps.satellites.age());
      } else {
        Serial.println(F("INVALID"));
      }

      iterationsCounter = 0;
      Serial.println();
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10){
    Serial.println(F("No GPS data received: check wiring"));
  }
  iterationsCounter ++;
  // delay(5000);
}
