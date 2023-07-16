#include <SoftwareSerial.h>
#include "CodecKT1.h"

const char* deviceId = "gBhmSbJlmIHuRbvgxfRajJTrQSGoZoZqJZDEPNZh";  //Must be 40 Bytes
char* imei = "0356307042441013";                                    //Must be 16 bytes  sprintf(imei, "%016s", imei);

float longitude = -1.349856;
float latitude = 32.455678;
uint64_t timestamp = 1689359519;
uint8_t satellites = 10;
uint16_t acceleration = 512;
uint8_t state = 1;
uint16_t battVoltage = 3600;

// TCP Variables
String apn = "Safaricom";
String server_ip = "151.80.209.133";
String server_port = "6500";

CodecKT1 codec;


void setup() {
  Serial.begin(9600);

  String deviceDataPacketString = codec.createDeviceDataPacket(longitude, latitude, timestamp, satellites, acceleration, state, battVoltage);
  Serial.println("Device Data Packet:");
  Serial.println(deviceDataPacketString);

  String deviceLoginPacketString = codec.createLoginPacket(imei, deviceId);
  Serial.println("Device Login Packet:");
  Serial.println(deviceLoginPacketString);

}

void loop() {

}


void checkLoginDataLenths(){
  /*
  A utility that can be used to check the length of the device_id and device_imei values.
  This function should be run every time a the begining of the loop.
  */

  size_t deviceIdLength = strlen(deviceId);
  size_t imeiLength = strlen(imei);

  if(deviceIdLength < 40 || imeiLength < 16){
    Serial.print(F("[CRITICAL] One of the Login Data parts has a length less than needed; "));
    Serial.print("Devive ID len: ");
    Serial.print(deviceIdLength);
    Serial.print(" IMEI len: ");
    Serial.println(imeiLength);
  }
  
}

