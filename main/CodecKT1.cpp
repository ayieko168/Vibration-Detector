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

String CodecKT1::createLoginPacket(const char* imei, const char* deviceId) {
  const int imeiLength = strlen(imei);
  const int deviceIdLength = strlen(deviceId);
  const size_t packetLength = imeiLength + deviceIdLength + 1;  // 1 byte for the protocol number
  int index1 = 0;
  int index2 = 0;

  String packetString = "";

  // Start bit
  packetString += "EEEE";

  // Packet length
  packetString += "40";

  // Protocol number
  packetString += "01";

  // Packet data (IMEI)
  char hexImei[33];
  for (int i = 0; i < 32 - 2 * imeiLength; i++) { hexImei[index1++] = '0'; }
  for (int i = 0; i < imeiLength; i++) {
    byte highNibble = (imei[i] >> 4) & 0x0F;
    byte lowNibble = imei[i] & 0x0F;
    hexImei[index1++] = (highNibble < 10) ? (highNibble + '0') : (highNibble - 10 + 'A');
    hexImei[index1++] = (lowNibble < 10) ? (lowNibble + '0') : (lowNibble - 10 + 'A');
  }
  hexImei[index1] = '\0';
  packetString += hexImei;
  // Serial.println(hexImei);

  // Packet data (Device ID)
  char hexDeviceId[81];
  for (int i = 0; i < 80 - 2 * deviceIdLength; i++) { hexDeviceId[index2++] = '0'; }
  for (int i = 0; i < deviceIdLength; i++) {
    byte highNibble = (deviceId[i] >> 4) & 0x0F;
    byte lowNibble = deviceId[i] & 0x0F;
    hexDeviceId[index2++] = (highNibble < 10) ? (highNibble + '0') : (highNibble - 10 + 'A');
    hexDeviceId[index2++] = (lowNibble < 10) ? (lowNibble + '0') : (lowNibble - 10 + 'A');
  }
  hexDeviceId[index2] = '\0';
  packetString += hexDeviceId;
  // Serial.println(hexDeviceId);

  // Calculate Checksum
  String info_packet = packetString.substring(8);
  uint8_t checksumData[info_packet.length() / 2];
  for (size_t i = 0; i < info_packet.length(); i += 2) {
    checksumData[i / 2] = (uint8_t)strtol(info_packet.substring(i, i + 2).c_str(), NULL, 16);
  }
  uint16_t crc = calculateCRC16(checksumData, sizeof(checksumData));
  char crcHex[5];
  snprintf(crcHex, sizeof(crcHex), "%04X", crc);
  packetString += crcHex;

  // Stop bit
  packetString += "AAAA";

  return packetString;
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
  // Serial.println(hexLongitude);

  // Latitude
  uint32_t latitudeData;
  memcpy(&latitudeData, &latitude, sizeof(latitudeData));
  char hexLatitude[sizeof(latitudeData) * 2 + 1];
  for (int i = sizeof(latitudeData) * 2 - 1; i >= 0; i--)
    hexLatitude[sizeof(latitudeData) * 2 - 1 - i] = "0123456789ABCDEF"[(latitudeData >> (i / 2 * 8 + 4 * (i % 2))) & 0xF];
  hexLatitude[sizeof(latitudeData) * 2] = '\0';
  packet_hex_string += hexLatitude;
  // Serial.println(hexLatitude);

  // Timestamp
  char hexTimestamp[17];
  for (int i = 0; i < 16; i++)
    hexTimestamp[i] = "0123456789ABCDEF"[(timestamp >> ((15 - i) * 4)) & 0xF];
  hexTimestamp[16] = '\0';
  packet_hex_string += hexTimestamp;
  // Serial.println(hexTimestamp);

  // Satellites
  char hexSatellites[3];
  snprintf(hexSatellites, sizeof(hexSatellites), "%02X", satellites);
  packet_hex_string += hexSatellites;
  // Serial.println(hexSatellites);

  // State
  char hexState[3];
  snprintf(hexState, sizeof(hexState), "%02X", state);
  packet_hex_string += hexState;
  // Serial.println(hexState);

  // Battery Voltage
  char hexBattVoltage[5];
  snprintf(hexBattVoltage, sizeof(hexBattVoltage), "%04X", battVoltage);
  packet_hex_string += hexBattVoltage;
  // Serial.println(hexBattVoltage);

  // Acceleration
  char hexAcceleration[5];
  snprintf(hexAcceleration, sizeof(hexAcceleration), "%04X", acceleration);
  packet_hex_string += hexAcceleration;
  // Serial.println(hexAcceleration);

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

String CodecKT1::createDeviceDataPacket(const char* imei, float longitude, float latitude, uint64_t timestamp, uint8_t satellites, uint16_t acceleration, uint8_t state, uint16_t battVoltage) {
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
  // Serial.println(hexLongitude);

  // Latitude
  uint32_t latitudeData;
  memcpy(&latitudeData, &latitude, sizeof(latitudeData));
  char hexLatitude[sizeof(latitudeData) * 2 + 1];
  for (int i = sizeof(latitudeData) * 2 - 1; i >= 0; i--)
    hexLatitude[sizeof(latitudeData) * 2 - 1 - i] = "0123456789ABCDEF"[(latitudeData >> (i / 2 * 8 + 4 * (i % 2))) & 0xF];
  hexLatitude[sizeof(latitudeData) * 2] = '\0';
  packet_hex_string += hexLatitude;
  // Serial.println(hexLatitude);

  // Timestamp
  char hexTimestamp[17];
  for (int i = 0; i < 16; i++)
    hexTimestamp[i] = "0123456789ABCDEF"[(timestamp >> ((15 - i) * 4)) & 0xF];
  hexTimestamp[16] = '\0';
  packet_hex_string += hexTimestamp;
  // Serial.println(hexTimestamp);

  // Packet data (IMEI)
  packet_hex_string += imei;
  // Serial.println(imei);

  // Satellites
  char hexSatellites[3];
  snprintf(hexSatellites, sizeof(hexSatellites), "%02X", satellites);
  packet_hex_string += hexSatellites;
  // Serial.println(hexSatellites);

  // State
  char hexState[3];
  snprintf(hexState, sizeof(hexState), "%02X", state);
  packet_hex_string += hexState;
  // Serial.println(hexState);

  // Battery Voltage
  char hexBattVoltage[5];
  snprintf(hexBattVoltage, sizeof(hexBattVoltage), "%04X", battVoltage);
  packet_hex_string += hexBattVoltage;
  // Serial.println(hexBattVoltage);

  // Acceleration
  char hexAcceleration[5];
  snprintf(hexAcceleration, sizeof(hexAcceleration), "%04X", acceleration);
  packet_hex_string += hexAcceleration;
  // Serial.println(hexAcceleration);

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

String CodecKT1::verifyAcknowledgmentPacket(const String& packet, const char* localDeviceId) {
  
  // Check if the packet length is valid
  if (packet.length() != 128) {
    return "1122"; // Invalid Packet, invalid length
  }

  // Check the start bit
  if (packet.substring(0, 4) != "7946") {
    return "1133"; // Invalid Packet, Invalid Start bit
  }

  // Extract the packet length
  int packetLength = strtol(packet.substring(4, 6).c_str(), NULL, 16);

  // Extract the protocol number
  int protocolNumber = strtol(packet.substring(6, 8).c_str(), NULL, 16);

  // Extract the server ID
  String serverID = packet.substring(8, 40);

  // Extract the device ID
  String deviceID = packet.substring(40, 120);
  const char* deviceIDChar = deviceID.c_str();

  // Convert the hexadecimal string to a byte array
  String hexString = packet.substring(8, 120);
  size_t dataSize = hexString.length() / 2;
  uint8_t data[dataSize];
  for (size_t i = 0; i < dataSize; i++) {
    sscanf(hexString.substring(i * 2, i * 2 + 2).c_str(), "%02X", &data[i]);
  }

  // Calculate the CRC16 (X.25) checksum of the byte array
  uint16_t calculatedCRC = calculateCRC16(data, dataSize);

  // Extract the received CRC from the packet
  String receivedCRCStr = packet.substring(120, 124);
  uint16_t receivedCRC = strtol(receivedCRCStr.c_str(), NULL, 16);

  // Check the CRC
  if (calculatedCRC != receivedCRC) {
    Serial.print("Calculated CRC: ");
    Serial.print(calculatedCRC);
    Serial.print(" Recieved CRC: ");
    Serial.println(receivedCRCStr);

    return "1155"; // Invalid Packet, invalid CRC Checksum
  }

  // Check the stop bit
  if (packet.substring(124, 128) != "6497") {
    return "1144"; // Invalid Packet, invalid Stop bit
  }

  // Check that the recieved device ID is similar to the local device ID
  char localDeviceIdString[41];
  for (int i = 0; i < 40; i++) { sprintf(localDeviceIdString + (i * 2), "%02X", localDeviceId[i]); }
  if (strcmp(localDeviceIdString, deviceIDChar) != 0){
    
    // Serial.println(localDeviceIdString);
    // Serial.println(deviceIDChar);

    return "1166"; // Invalid Packet, disimilar device ID
  }

  // Return success
  return "1111"; // Success
}

bool CodecKT1::validateLoginPacket(const String &packet) {
  // Check if the packet is 128 characters long
  if (packet.length() != 128) {
    return false;
  }

  // Check if the first 4 letters are "EEEE"
  if (packet.substring(0, 4) != "EEEE") {
    return false;
  }

  // Check if the last 4 letters are "AAAA"
  if (packet.substring(124) != "AAAA") {
    return false;
  }

  // If all checks pass, the packet is valid
  return true;
}








