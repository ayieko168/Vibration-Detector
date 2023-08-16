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


void setup() {
  Serial.begin(9600);

  // Hexadecimal string to calculate CRC for
  String hexString = "421321EABFA552530000000064B1949F086471303026844808000E100000";

  // Convert the hexadecimal string to a byte array
  size_t dataSize = hexString.length() / 2;
  uint8_t data[dataSize];
  for (size_t i = 0; i < dataSize; i++) {
    sscanf(hexString.substring(i * 2, i * 2 + 2).c_str(), "%02X", &data[i]);
  }

  // Calculate the CRC16 (X.25) checksum
  uint16_t crc = calculateCRC16(data, dataSize);

  // Print the CRC value
  Serial.print("CRC: 0x");
  if (crc < 0x1000) Serial.print("0");
  if (crc < 0x100) Serial.print("0");
  Serial.println(crc, HEX);
}

void loop() {
}
