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

  // Serial.print("Check Sum Data #1: ");
  // Serial.println(String(crc));

  return crc;
}

String CodecKT1::createDeviceDataPacket(const String imei, const float longitude, const float latitude, const uint64_t timestamp, const uint8_t satellites, const uint16_t acceleration, const uint8_t state, const uint16_t battVoltage) {
  String packet_hex_string = "";
  // packet_hex_string.reserve(70);

  // Start bit
  packet_hex_string.concat("EEEE");

  // Packet Length
  packet_hex_string.concat("16");


  // Protocol Number
  packet_hex_string.concat("01");

  // Information bits
  // Longitude
  uint32_t longitudeData;
  memcpy(&longitudeData, &longitude, sizeof(longitudeData));
  char hexLongitude[9];
  snprintf(hexLongitude, sizeof(hexLongitude), "%08X", longitudeData);
  packet_hex_string.concat(hexLongitude);
  // Serial.println(packet_hex_string);

  // Latitude
  uint32_t latitudeData;
  memcpy(&latitudeData, &latitude, sizeof(latitudeData));
  char hexLatitude[9];
  snprintf(hexLatitude, sizeof(hexLatitude), "%08X", latitudeData);
  packet_hex_string.concat(hexLatitude);
  // Serial.println(packet_hex_string);

  // Timestamp
  char hexTimestamp[17];
  snprintf(hexTimestamp, sizeof(hexTimestamp), "%016llX", static_cast<unsigned long long>(timestamp));
  packet_hex_string.concat(hexTimestamp);
  // Serial.println(packet_hex_string);

  // IMEI
  packet_hex_string.concat(imei);
  // Serial.println(packet_hex_string);

  // Satellites
  char hexSatellites[3];
  snprintf(hexSatellites, sizeof(hexSatellites), "%02X", satellites);
  packet_hex_string.concat(hexSatellites);
  // Serial.println(packet_hex_string);

  // State
  char hexState[3];
  snprintf(hexState, sizeof(hexState), "%02X", state);
  packet_hex_string.concat(hexState);
  // Serial.println(packet_hex_string);

  // Battery Voltage
  char hexBattVoltage[5];
  snprintf(hexBattVoltage, sizeof(hexBattVoltage), "%04X", battVoltage);
  packet_hex_string.concat(hexBattVoltage);
  // Serial.println(packet_hex_string);

  // Acceleration
  char hexAcceleration[5];
  snprintf(hexAcceleration, sizeof(hexAcceleration), "%04X", acceleration);
  packet_hex_string.concat(hexAcceleration);
  // Serial.println(packet_hex_string);

  // Calculate Checksum
  // String info_packet = packet_hex_string.substring(8);
  // size_t dataSize = info_packet.length() / 2;
  // uint8_t data[dataSize];
  // for (size_t i = 0; i < dataSize; i++) {
  //   sscanf(info_packet.substring(i * 2, i * 2 + 2).c_str(), "%02X", &data[i]);
  // }
  
  uint16_t crc = 0; //calculateCRC16(data, dataSize);
  char crcHex[5];
  snprintf(crcHex, sizeof(crcHex), "%04X", crc);
  packet_hex_string.concat(crcHex);
  // Serial.print(" checksumData: ");
  // Serial.println(String(crcHex));

  // Stop bit
  packet_hex_string.concat("AAAA");

  return packet_hex_string;
}







