#include <TinyGPS++.h>
#include "CodecKT1.h"
#include "TCPComms.h"

char* imei = "NULL0";
String deviceLoginPacketString = "NULL";
const int BOARD_RESET_PIN = 2;
String serverResponse;

/* GPS Module Variables */
const int GPSRXPin = 8, GPSTXPin = 7;
const uint32_t GPSBaud = 9600;
int dataAquisitionRetries = 0;

float longitude = -1.349856;
float latitude = 32.455678;
uint64_t timestamp = 1689359519;
uint8_t satellites = 10;
uint16_t acceleration = 512;
uint8_t state = 1;
uint16_t battVoltage = 3600;

int connectedToInterent;

CodecKT1 codec;
TCPComms tcpcoms;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPSRXPin, GPSTXPin);


void setup() {

  digitalWrite(BOARD_RESET_PIN, HIGH);
  pinMode(BOARD_RESET_PIN, OUTPUT); 

  /* ------------- Initialize the serial ports ---------------- */
  // The USB serial
  Serial.begin(9600);

  // The SIM8000 Serial
  bool tcpcoms_started = false;
  while (!tcpcoms_started){
    tcpcoms_started = tcpcoms.begin(10, 11, 12);
    Serial.print("SIM800 Serial Connected: ");
    Serial.println(tcpcoms_started);
    delay(100);
  }

  /* --------- Set the IMEI Number from the device ----------*/
  String newImeiString = tcpcoms.getImeiNumber();
  imei = strdup(newImeiString.c_str());
  Serial.print("IMEI Number set to: ");
  Serial.println(imei);

  /* ------------- Set the datetime to current network datetime -------------*/
  int timeSetState = tcpcoms.setCurrentTime();
  Serial.print("Setting time to current datetime: ");
  Serial.println(timeSetState);

  /* ------------- Get the current date ----------------*/
  unsigned long currentTimeStamp = tcpcoms.getTimestamp();
  Serial.print("Current Timestamp: ");
  Serial.println(currentTimeStamp);

  /* ----------------- GPS Sensor ------------------*/

  /* -------------- Connect to the Internet ----------------*/
  // int connectionStatus = -1;
  // int connectionRetries = 0;
  // while (connectionStatus != 200 && connectionStatus != 202){
  //   connectionStatus = tcpcoms.connectInternet();
  //   Serial.print("Internet Connection Status: ");
  //   Serial.println(connectionStatus); 
  //   delay(5000);

  //   connectionRetries ++;
  //   if (connectionRetries >= 5){
  //     Serial.println("Resetting the SIM800 and retrying in 10 Seconds...");
  //     tcpcoms.resetSim800();
  //     connectionRetries = 0;
  //     delay(10000);
  //   }
  // }

  // GPS Serial
  gpsSerial.begin(GPSBaud);

}

void loop() {

  /* ------------------ Extract the variable values from the sensors and gsm -----------------*/
  // GPS Sensor
  for (int gpsReadRuns = 0; gpsReadRuns < 500; gpsReadRuns++){  // Try reading 500 times and break immedietly the GPS send me some data
    if (gpsSerial.available() > 0) {

      if (gps.encode(gpsSerial.read())) {

        if (gps.location.isValid()) {
          latitude = static_cast<float>(gps.location.lat());
          Serial.print(F("- latitude: "));
          Serial.println(gps.location.lat(), 8);

          longitude = static_cast<float>(gps.location.lng());
          Serial.print(F("- longitude: "));
          Serial.println(gps.location.lng(), 8);

        } else {
          longitude = 0;
          latitude = 0;
          Serial.println(F("- location: INVALID"));
        }

        Serial.print(F("- speed: "));
        if (gps.speed.isValid()) {

          acceleration = static_cast<uint16_t>(gps.speed.kmph());
          Serial.print(gps.speed.kmph());
          Serial.println(F(" km/h"));
        } else {
          acceleration = 0;
          Serial.println(F("INVALID"));
        }

        Serial.print(F("- Satellites: "));
        if (gps.satellites.isValid()) {

          uint32_t sats_32 = gps.satellites.value();
          if (sats_32 <= UINT8_MAX) {
            // Value can be safely represented in uint8_t range
            satellites = static_cast<uint8_t>(sats_32);
          } else {
            // Value exceeds the uint8_t range, handle the overflow or truncation
            // For example, you may choose to saturate the value to the maximum representable value
            satellites = 0;
          }
          Serial.print(gps.satellites.value());
        } else {

          satellites = UINT8_MAX;
          Serial.println(F("INVALID"));
        }

        Serial.println();
        Serial.print("GPS Data Aquisition Retries: ");
        Serial.println(dataAquisitionRetries);
        dataAquisitionRetries = 0;
        break;
      }
    }

    // if (millis() > 5000 && gps.charsProcessed() < 10){
    //   Serial.println(F("No GPS data received: check wiring"));
    // }

    dataAquisitionRetries ++;
  }

  // 

  /* --------------- Send the data packet -----------------*/
  // serverResponse = "";
  String deviceDataPacketString = codec.createDeviceDataPacket(imei, longitude, latitude, timestamp, satellites, acceleration, state, battVoltage);
  // String sentErrorCheck = deviceDataPacketString.substring(68, 72);
  // serverResponse = tcpcoms.sendDataWithResponse(deviceDataPacketString);
  Serial.println(deviceDataPacketString);
  // Serial.print("Recieved Error Check: ");
  // Serial.print(serverResponse);
  // Serial.print(" Sent Error Cech: ");
  // Serial.println(sentErrorCheck);

  // delay(5000);  // Wait X seconds before sending another packet.
  Serial.println("==================================");
}


