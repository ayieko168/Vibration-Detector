#include <TinyGPS++.h>
#include <Wire.h>
#include <MPU6050.h>
#include "CodecKT1.h"
#include "TCPComms.h"

String imei = "NONE0";
String deviceLoginPacketString = "NULL";
const int BOARD_RESET_PIN = 2;
String serverResponse;
int connectedToInterent;

/* GPS Module Variables */
const int GPSRXPin = 6, GPSTXPin = 5;
const uint32_t GPSBaud = 9600;
int dataAquisitionRetries = 0;
bool foundValidData = false;


/* Gyro Module Variables (MPU6050) */
uint8_t MPU6050_GYRO_FS_250 = 0;
uint8_t MPU6050_GYRO_FS_500 = 1;
uint8_t MPU6050_GYRO_FS_1000 = 2;
uint8_t MPU6050_GYRO_FS_2000 = 3;
float previousReading = 0.0;
float absoluteDisplasement = 0.0;
float gyroscopeMagnitude = 0.0;
float WORKING_THRESHOLD_DISP = 50.0;  // Amplitude
float IDLE_THRESHOLD_DISP = 5.0;
float OFF_THRESHOLD_DISP = 0.0;
int WORKING_THRESHOLD_PERIOD = 5000;  // In MilliSeconds
int IDLE_THRESHOLD_PERIOD = 5000;
int OFF_THRESHOLD_PERIOD = 60000;
unsigned long currentMillis_w;
unsigned long previousMillis_w;
unsigned long currentMillis_i;
unsigned long previousMillis_i;

/* Packet Variables */
float longitude = -1.349856;
float latitude = 32.455678;
uint64_t timestamp = 1689359519;
uint8_t satellites = 10;
uint16_t acceleration = 512;
uint8_t state = 0;
uint16_t battVoltage = 3600;

CodecKT1 codec;
TCPComms tcpcoms;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPSRXPin, GPSTXPin);
MPU6050 mpu;


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
  while (imei.indexOf("NONE") != -1){
    imei = tcpcoms.getImeiNumber();
    Serial.print("IMEI Number set to: ");
    Serial.println(imei);
    delay(100);
  }

  /* ------------- Set the datetime to current network datetime -------------*/
  int timeSetState = 999;
  while (timeSetState != 200 && timeSetState != 201){
    timeSetState = tcpcoms.setCurrentTime();
    Serial.print("Setting time to current datetime: ");
    Serial.println(timeSetState);
  }

  /* ------------- Get the current date ----------------*/
  unsigned long currentTimeStamp = 99;
  while (currentTimeStamp == 99) {
    currentTimeStamp = tcpcoms.getTimestamp();
    Serial.print("Current Timestamp: ");
    Serial.println(currentTimeStamp);
  }

  /* ----------------- GYRO Sensor ------------------*/
  // Initialize the MPU6050 sensor
  Wire.begin();
  mpu.initialize();

  // Set gyroscope full-scale range (FSR)
  // Options: MPU6050_GYRO_FS_250, MPU6050_GYRO_FS_500, MPU6050_GYRO_FS_1000, MPU6050_GYRO_FS_2000 (Higher the value the more sensitive the Gyro)
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  Serial.print("GYRO Full range: ");
  Serial.println(mpu.getFullScaleGyroRange());

  /* -------------- Connect to the Internet ----------------*/
  int connectionStatus = -1;
  int connectionRetries = 0;
  while (connectionStatus != 200 && connectionStatus != 202){
    connectionStatus = tcpcoms.connectInternet();
    Serial.print("Internet Connection Status: ");
    Serial.println(connectionStatus); 
    delay(5000);

    connectionRetries ++;
    if (connectionRetries >= 5){
      Serial.println("Resetting the SIM800 and retrying in 10 Seconds...");
      tcpcoms.resetSim800();
      connectionRetries = 0;
      delay(10000);
    }
  }

  // GPS Serial
  gpsSerial.begin(GPSBaud);

}

void loop() {

  /* ------------------ Extract the variable values from the sensors and gsm -----------------*/
  // Read gyroscope data for a second
  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  // Calculate the magnitude of the gyroscope vector
  float gyroscopeMagnitude = sqrt(gx * gx + gy * gy + gz * gz);

  // Check if the gyroscope magnitude is a valid number
  if (isnan(gyroscopeMagnitude)) {
    // Skip the rest of the loop and wait for the next reading
    return;
  }

  /* Calculate absolute displacement */
  absoluteDisplasement = abs(previousReading - gyroscopeMagnitude);
  float absDisp2 = absoluteDisplasement * absoluteDisplasement;

  /* Logic to check if state has changed */
  // if (absDisp2 > WORKING_THRESHOLD_DISP) {
  //   // In Working state
  //   currentMillis_w = millis();
  //   if (abs(currentMillis_w - previousMillis_w) > WORKING_THRESHOLD_PERIOD) {
  //     Serial.print(abs(currentMillis_w - previousMillis_w));
  //     Serial.println(" WORKING STATE! ");
  //     // Set state
  //     state = 2;
  //     previousMillis_w = currentMillis_w;
  //   }
  //   // Reset the idle state timer
  //   previousMillis_i = millis();
  // } else if (absDisp2 > IDLE_THRESHOLD_DISP && absDisp2 < WORKING_THRESHOLD_DISP) {
  //   // In Idle state
  //   currentMillis_i = millis();
  //   if (abs(currentMillis_i - previousMillis_i) > IDLE_THRESHOLD_PERIOD) {
  //     Serial.print(abs(currentMillis_i - previousMillis_i));
  //     Serial.println(" IDLE STATE! ");
  //     // Set state
  //     state = 1;
  //     previousMillis_i = currentMillis_i;
  //     previousMillis_w = millis();
  //   }
  // } else {
  //   Serial.println(" OFF STATE! ");
  //     // Set state
  //   state = 3;

  //   currentMillis_w = 0;
  //   previousMillis_w = 0;
  //   currentMillis_i = 0;
  //   previousMillis_i = 0;
  // }

  // Set current reading to the previous value for the next iteration.
  previousReading = gyroscopeMagnitude;


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

          // Break out of the Location Loop when a valip location is aquired.
          foundValidData = true;
          // break;

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
      }

      if (foundValidData){
        Serial.println();
        Serial.print("GPS Data Aquisition Retries: ");
        Serial.println(dataAquisitionRetries);
        dataAquisitionRetries = 0;
        foundValidData = false;
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
  serverResponse = "";
  String deviceDataPacketString = codec.createDeviceDataPacket(imei, longitude, latitude, timestamp, satellites, acceleration, state, battVoltage);
  if (deviceDataPacketString.length() != 72){
    Serial.print("Device Data Packet String: ");
    Serial.println(deviceDataPacketString);

    Serial.println("==================================");
    delay(5000);
    return;
    
  }
  // String sentErrorCheck = deviceDataPacketString.substring(68, 72);
  // serverResponse = tcpcoms.sendDataWithResponse(deviceDataPacketString);
  Serial.print("Device Data Packet String: ");
  Serial.println(deviceDataPacketString);
  // Serial.print("Recieved Error Check: ");
  // Serial.print(serverResponse);
  // Serial.print(" Sent Error Cech: ");
  // Serial.println(sentErrorCheck);

  Serial.println("==================================");
  delay(5000);  // Wait X seconds before sending another packet.
}


