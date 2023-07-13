#include "CodecKT1.h"

void CodecKT1::begin(const char* deviceId, const char* imei) {

}

uint16_t CodecKT1::calculateCRC16(const uint8_t* data, size_t length) {
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i] << 8;

    for (int j = 0; j < 8; ++j) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }

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
  uint16_t crc = calculateCRC16(packet - packetLength, packetLength);
  *packet++ = crc >> 8;
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
  memcpy(packet, &longitude, sizeof(longitude));
  packet += sizeof(longitude);
  memcpy(packet, &latitude, sizeof(latitude));
  packet += sizeof(latitude);
  memcpy(packet, &timestamp, sizeof(timestamp));
  packet += sizeof(timestamp);
  *packet++ = satellites;
  memcpy(packet, &acceleration, sizeof(acceleration));
  packet += sizeof(acceleration);
  *packet++ = state;
  memcpy(packet, &battVoltage, sizeof(battVoltage));
  packet += sizeof(battVoltage);

  // Error Check
  uint16_t crc = calculateCRC16(packet - 0x17, 0x17);
  *packet++ = crc >> 8;
  *packet++ = crc & 0xFF;

  // Stop bit
  *packet++ = 0xaa;
  *packet++ = 0xaa;
}


