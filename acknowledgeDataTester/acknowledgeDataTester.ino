
void setup() {
  Serial.begin(9600);

  String packet = "794638036CF1D74BCF6ED96F5A4FCA3F771DAEAFC4F8FFA6C3198EF6F9DDAAA850A646583D030C00E515D5BEA2C5BBA900C6B71EFFCC335E4B2AD09302306497";
  String runStatus = decodePacket(packet);
  Serial.print("Response Code: ");
  Serial.println(runStatus);
}

void loop() {
  // Empty loop
}


uint16_t calculateCRC16(uint8_t* data, size_t dataSize) {
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

String decodePacket(const String& packet) {

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

  // Convert server ID and device ID to uint8_t array for CRC calculation
  uint8_t data[56];
  for (int i = 0; i < 16; i++) {
    String byteStr = packet.substring(8 + i * 4, 12 + i * 4);
    uint16_t byteVal = strtol(byteStr.c_str(), NULL, 16);
    data[i * 2] = byteVal >> 8;
    data[i * 2 + 1] = byteVal & 0xFF;
  }
  for (int i = 0; i < 40; i++) {
    String byteStr = packet.substring(40 + i * 2, 42 + i * 2);
    data[32 + i] = strtol(byteStr.c_str(), NULL, 16);
  }

  // Calculate the CRC
  uint16_t calculatedCRC = calculateCRC16(data, 56);

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

  // Return success
  return "1111"; // Success
}

