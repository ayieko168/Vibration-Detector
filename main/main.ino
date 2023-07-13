#include <SoftwareSerial.h>
#include "CodecKT1.h"

const char* imei = "1234567812345678";
const char* deviceId = "ABCDEF1234567890";

float longitude = -1.349856;
float latitude = 32.455678;
uint64_t timestamp = 1624907412;
uint8_t satellites = 10;
uint16_t acceleration = 512;
uint8_t state = 1;
uint16_t battVoltage = 3600;

// Assuming a buffer of size 100 to hold the packets
uint8_t loginPacket[56];
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
  
  Serial.println("Login Packet:");
  for (size_t i = 0; i < sizeof(loginPacket); ++i) {
    // Serial.print("0x");
    Serial.print(loginPacket[i], HEX);
    Serial.print(" ");
  }
  Serial.println();


}

void loop() {
  // put your main code here, to run repeatedly:

}
