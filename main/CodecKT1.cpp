#include "CodecKT1.h"

uint16_t CodecKT1::calculateCRC16(uint8_t* data, size_t dataSize) {
  const uint16_t polynomial = 0x8408;
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < dataSize; i++) {
    crc ^= data[i];

    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ polynomial;
      else
        crc >>= 1;
    }
  }

  crc ^= 0xFFFF;  // Bitwise XOR with 0xFFFF
  return crc;
}

void CodecKT1::createLoginPacket(uint8_t* packet, const char* imei, const char* deviceId) {
  size_t imeiLength = strlen(imei);
  size_t deviceIdLength = strlen(deviceId);
  size_t packetLength = imeiLength + deviceIdLength + 1;  // 1 byte for the protocol number

  // Start bit
  *packet++ = 0xee;
  *packet++ = 0xee;

  // Packet length
  *packet++ = static_cast<uint8_t>(packetLength);

  // Protocol number
  *packet++ = 0x01;

  // Packet data (IMEI and Device ID)
  memcpy(packet, imei, imeiLength);
  packet += imeiLength;
  memcpy(packet, deviceId, deviceIdLength);
  packet += deviceIdLength;

  // Error Check
  uint16_t crc = calculateCRC16(packet - packetLength + 1, packetLength - 1);
  *packet++ = (crc >> 8) & 0xFF;
  *packet++ = crc & 0xFF;

  // Stop bit
  *packet++ = 0xaa;
  *packet++ = 0xaa;
}

void CodecKT1::createDeviceDataPacket(uint8_t* packet, float longitude, float latitude, uint64_t timestamp, uint8_t satellites, uint16_t acceleration, uint8_t state, uint16_t battVoltage) {
  // Start bit
  *packet++ = 0xee;
  *packet++ = 0xee;

  // Packet length
  *packet++ = 0x17;

  // Protocol number
  *packet++ = 0x02;

  // Packet data
  int32_t longitudeFixed = static_cast<int32_t>(longitude * 1000000);  // Convert float to fixed-point with 6 decimal places
  memcpy(packet, &longitudeFixed, 4);  // 4 bytes for longitude
  packet += 4;

  int32_t latitudeFixed = static_cast<int32_t>(latitude * 1000000);  // Convert float to fixed-point with 6 decimal places
  memcpy(packet, &latitudeFixed, 4);  // 4 bytes for latitude
  packet += 4;

  memcpy(packet, &timestamp, 8);  // 8 bytes for timestamp
  packet += 8;

  *packet++ = satellites;  // 1 byte for satellites

  int16_t accelerationFixed = static_cast<int16_t>(acceleration * 100);  // Convert float to fixed-point with 2 decimal places
  memcpy(packet, &accelerationFixed, 2);  // 2 bytes for acceleration
  packet += 2;

  *packet++ = state;  // 1 byte for state

  uint16_t battVoltageFixed = static_cast<uint16_t>(battVoltage * 100);  // Convert float to fixed-point with 2 decimal places
  memcpy(packet, &battVoltageFixed, 2);  // 2 bytes for battVoltage
  packet += 2;

  // Error Check
  uint16_t crc = calculateCRC16(packet - packetLength + 1, packetLength - 1);
  *packet++ = crc & 0xFF;
  *packet++ = (crc >> 8) & 0xFF;

  // Stop bit
  *packet++ = 0xaa;
  *packet++ = 0xaa;
}






