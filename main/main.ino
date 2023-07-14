#include <SoftwareSerial.h>
#include "CodecKT1.h"

const char* deviceId = "gBhmSbJlmIHuRbvgxfRajJTrQSGoZoZqJZDEPNZT";  //Must be 40 Bytes
char* imei = "0356307042441013";                                    //Must be 8 bytes  sprintf(imei, "%016s", imei);

float longitude = -1.349856;
float latitude = 32.455678;
uint64_t timestamp = 1624907412;
uint8_t satellites = 10;
uint16_t acceleration = 512;
uint8_t state = 1;
uint16_t battVoltage = 3600;

// Assuming a buffer of size 100 to hold the packets
uint8_t loginPacket[64];
uint8_t deviceDataPacket[100];

// TCP Variables
String apn = "Safaricom";
String server_ip = "151.80.209.133";
String server_port = "6500";

CodecKT1 codec;


void setup() {
  Serial.begin(9600);
  codec.begin(deviceId, imei);

  codec.createLoginPacket(loginPacket, imei, deviceId);
  size_t bufferSize = sizeof(loginPacket) / sizeof(loginPacket[0]);
  
  Serial.println("Login Packet:");
  String loginPacketString = bufferToString(loginPacket, bufferSize);
  Serial.println(loginPacketString);

  checkLoginDataLenths();

}

void loop() {
  // put your main code here, to run repeatedly:

}

String bufferToString(uint8_t* buffer, size_t bufferSize) {
  String result;
  for (size_t i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) {
      result += "0";  // Zero-padding if necessary
    }
    result += String(buffer[i], HEX);  // Append the hexadecimal value
  }
  return result;
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

