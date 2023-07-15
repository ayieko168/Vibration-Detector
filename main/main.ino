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

// Set asside space for the data packets
uint8_t loginPacket[64];
uint8_t deviceDataPacket[30];

// TCP Variables
String apn = "Safaricom";
String server_ip = "151.80.209.133";
String server_port = "6500";

CodecKT1 codec;


void setup() {
  Serial.begin(9600);

  // Test Login Packet
  // codec.createLoginPacket(loginPacket, imei, deviceId);
  // size_t loginPacketSize = sizeof(loginPacket) / sizeof(loginPacket[0]);
  // Serial.println("Login Packet:");
  // String loginPacketString = bufferToString(loginPacket, loginPacketSize);
  // Serial.println(loginPacketString);


  // Test Device data packet
  // String deviceDataPacketString = codec.createDeviceDataPacket(longitude, latitude, timestamp, satellites, acceleration, state, battVoltage);
  // Serial.println("Device Data Packet:");
  // Serial.println(deviceDataPacketString);


  // Serial.print("0x");
  // Serial.println(*(unsigned long*)&longitude, HEX);

  // checkLoginDataLenths();

  /*   Test CRC function   */
  // String hexString = "E0981400FE3BEF01941EDA60000000000A01407E00C8";
  // size_t dataSize = hexString.length() / 2;
  // uint8_t data[dataSize];
  // for (size_t i = 0; i < dataSize; i++) {
  //   sscanf(hexString.substring(i * 2, i * 2 + 2).c_str(), "%02X", &data[i]);
  // }
  // uint16_t crc = codec.calculateCRC16(data, dataSize);
  // Serial.print("CRC: 0x");
  // if (crc < 0x1000) Serial.print("0");
  // if (crc < 0x100) Serial.print("0");
  // Serial.println(crc, HEX);

}

void loop() {
  
  while (!Serial.available() || Serial.read() != 'n') {
    // Do nothing until 'n' is entered
  }

  /* ---------- Test 2 for the data packet --------------- */

  // Print the assigned values
  // Print the assigned values
  Serial.print("Longitude: ");
  Serial.println(longitude);
  Serial.print("Latitude: ");
  Serial.println(latitude);
  Serial.print("Timestamp: ");
  Serial.println(String((unsigned long)timestamp));
  Serial.print("Satellites: ");
  Serial.println(satellites);
  Serial.print("Acceleration: ");
  Serial.println(acceleration);
  Serial.print("State: ");
  Serial.println(state);
  Serial.print("Battery Voltage: ");
  Serial.println(battVoltage);


  String deviceDataPacketString = codec.createDeviceDataPacket(longitude, latitude, timestamp, satellites, acceleration, state, battVoltage);
  Serial.println("Device Data Packet:");
  Serial.println(deviceDataPacketString);

  // Seed the random number generator
  randomSeed(analogRead(A0));

  // Assign random values to the variables
  longitude = randomFloat(-180.0, 180.0);
  latitude = randomFloat(-90.0, 90.0);
  timestamp = randomLong(0, 4294967295);
  satellites = random(1, 16);
  acceleration = random(0, 65535);
  state = random(0, 256);
  battVoltage = random(0, 16);

  /* -------------- END OF TEST 2 FOR DATA PACKET --------------------- */

  Serial.println();
  Serial.println("####################################################################");
  Serial.println();
}

float randomFloat(float minVal, float maxVal) {
  return minVal + (float)random() / ((float)RAND_MAX / (maxVal - minVal));
}

uint64_t randomLong(uint64_t minVal, uint64_t maxVal) {
  uint64_t randomValue = 0;
  for (int i = 0; i < sizeof(uint64_t); i++) {
    randomValue <<= 8;
    randomValue |= random(0, 256);
  }
  return randomValue % (maxVal - minVal + 1) + minVal;
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

