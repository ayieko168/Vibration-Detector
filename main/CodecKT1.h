#ifndef CODECKT1_H
#define CODECKT1_H

#include <Arduino.h>

class CodecKT1 {

  public:
    uint16_t calculateCRC16(uint8_t* data, size_t dataSize);
    String createDeviceDataPacket(const String imei, const float longitude, const float latitude, const uint64_t timestamp, const uint8_t satellites, const uint16_t acceleration, const uint8_t state, const uint16_t battVoltage);
    
};

#endif