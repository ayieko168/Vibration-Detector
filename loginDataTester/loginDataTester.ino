#include <SoftwareSerial.h>
#include "CodecKT1.h"

char* deviceId = "gBhmSbJlmIHuRbvgxfRajJTrQSGoZoZqJZDEPNZh";  //Must be 40 Bytes
char* imei = "0356307042441013";                                    //Must be 16 bytes  sprintf(imei, "%016s", imei);

CodecKT1 codec;

void setup() {
  Serial.begin(9600);
}

void loop() {
  while (!Serial.available() || Serial.read() != 'n') {
    // Do nothing until 'n' is entered
  }

  Serial.print("Device ID: ");
  Serial.println(deviceId);
  Serial.print("IMEI: ");
  Serial.println(imei);

  String deviceLoginPacketString = codec.createLoginPacket(imei, deviceId);
  Serial.println("Device Login Packet:");
  Serial.println(deviceLoginPacketString);

  // Generate random deviceId
  generateRandomDeviceId();

  // Generate random imei
  generateRandomImei();

  Serial.println("####################################################################");
  Serial.println();

}


void generateRandomDeviceId() {
  char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  int charsetSize = sizeof(charset) - 1;

  for (int i = 0; i < 40; i++) {
    deviceId[i] = charset[random(charsetSize)];
  }
  deviceId[40] = '\0';  // Null terminator
}

void generateRandomImei() {
  char charset[] = "0123456789";
  int charsetSize = sizeof(charset) - 1;

  for (int i = 0; i < 15; i++) {
    imei[i] = charset[random(charsetSize)];
  }
  imei[15] = '\0';  // Null terminator
}
