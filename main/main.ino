#include <SoftwareSerial.h>
#include "CodecKT1.h"

const char* deviceId = "gBhmSbJlmIHuRbvgxcRajJTrQSGoZoZqJZDEPNDJ";  //Must be 40 Bytes
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

  // codec.createLoginPacket(loginPacket, imei, deviceId);
  
  // Serial.println("Login Packet:");
  // for (size_t i = 0; i < sizeof(loginPacket); ++i) {
  //   // Serial.print("0x");
  //   Serial.print(loginPacket[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println();

  // Data to calculate CRC for
  uint8_t data[] = {0x39, 0x01, 0x30, 0x33, 0x35, 0x36, 0x33, 0x30, 0x37, 0x30, 0x34, 0x32, 0x34, 0x34, 0x31, 0x30, 0x31, 0x33, 0x67, 0x42, 0x68, 0x6D, 0x53, 0x62, 0x4A, 0x6C, 0x6D, 0x49, 0x48, 0x75, 0x52, 0x62, 0x76, 0x67, 0x78, 0x63, 0x52, 0x61, 0x6A, 0x4A, 0x54, 0x72, 0x51, 0x53, 0x47, 0x6F, 0x5A, 0x6F, 0x5A, 0x71, 0x4A, 0x5A, 0x44, 0x45, 0x50, 0x4E, 0x44, 0x4A};
  size_t dataSize = sizeof(data) / sizeof(data[0]);

  // Calculate the CRC32 checksum
  uint16_t crc = codec.calculateCRC16(data, dataSize);

  // Print the CRC value
  Serial.print("CRC: 0x");
  if (crc < 0x1000) Serial.print("0");
  if (crc < 0x100) Serial.print("0");
  Serial.println(crc, HEX);


}

void loop() {
  // put your main code here, to run repeatedly:

}
