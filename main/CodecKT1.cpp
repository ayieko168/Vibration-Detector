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

String CodecKT1::createDeviceDataPacket(float longitude, float latitude, uint64_t timestamp, uint8_t satellites, uint16_t acceleration, uint8_t state, uint16_t battVoltage) {
  String packet_hex_string = "";

  // Start bit
  packet_hex_string += "EEEE";

  // Packet Length
  packet_hex_string += "16";

  // Protocol Number
  packet_hex_string += "02";

  // Information bits
  // Longitude
  uint32_t longitudeData;
  memcpy(&longitudeData, &longitude, sizeof(longitudeData));
  char hexLongitude[sizeof(longitudeData) * 2 + 1];
  for (int i = sizeof(longitudeData) * 2 - 1; i >= 0; i--)
    hexLongitude[sizeof(longitudeData) * 2 - 1 - i] = "0123456789ABCDEF"[(longitudeData >> (i / 2 * 8 + 4 * (i % 2))) & 0xF];
  hexLongitude[sizeof(longitudeData) * 2] = '\0';
  packet_hex_string += hexLongitude;
  Serial.println(hexLongitude);

  // Latitude
  uint32_t latitudeData;
  memcpy(&latitudeData, &latitude, sizeof(latitudeData));
  char hexLatitude[sizeof(latitudeData) * 2 + 1];
  for (int i = sizeof(latitudeData) * 2 - 1; i >= 0; i--)
    hexLatitude[sizeof(latitudeData) * 2 - 1 - i] = "0123456789ABCDEF"[(latitudeData >> (i / 2 * 8 + 4 * (i % 2))) & 0xF];
  hexLatitude[sizeof(latitudeData) * 2] = '\0';
  packet_hex_string += hexLatitude;
  Serial.println(hexLatitude);

  // Timestamp
  char hexTimestamp[17];
  for (int i = 0; i < 16; i++)
    hexTimestamp[i] = "0123456789ABCDEF"[(timestamp >> ((15 - i) * 4)) & 0xF];
  hexTimestamp[16] = '\0';
  packet_hex_string += hexTimestamp;
  Serial.println(hexTimestamp);

  // Satellites
  char hexSatellites[3];
  snprintf(hexSatellites, sizeof(hexSatellites), "%02X", satellites);
  packet_hex_string += hexSatellites;
  Serial.println(hexSatellites);

  // State
  char hexState[3];
  snprintf(hexState, sizeof(hexState), "%02X", state);
  packet_hex_string += hexState;
  Serial.println(hexState);

  // Battery Voltage
  char hexBattVoltage[5];
  snprintf(hexBattVoltage, sizeof(hexBattVoltage), "%04X", battVoltage);
  packet_hex_string += hexBattVoltage;
  Serial.println(hexBattVoltage);

  // Acceleration
  char hexAcceleration[5];
  snprintf(hexAcceleration, sizeof(hexAcceleration), "%04X", acceleration);
  packet_hex_string += hexAcceleration;
  Serial.println(hexAcceleration);

  // Calculate Checksum
  String info_packet = packet_hex_string.substring(8);
  uint8_t checksumData[info_packet.length() / 2];
  for (size_t i = 0; i < info_packet.length(); i += 2) {
    checksumData[i / 2] = (uint8_t)strtol(info_packet.substring(i, i + 2).c_str(), NULL, 16);
  }
  uint16_t crc = calculateCRC16(checksumData, sizeof(checksumData));
  char crcHex[5];
  snprintf(crcHex, sizeof(crcHex), "%04X", crc);
  packet_hex_string += crcHex;
  // Serial.println(checksumData);
  // Serial.println(crcHex);


  // Stop bit
  packet_hex_string += "AAAA";

  return packet_hex_string;
}








