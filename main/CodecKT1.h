#ifndef CODECKT1_H
#define CODECKT1_H

#include <Arduino.h>

class CodecKT1 {

  public:
    // const char* deviceId;
    // const char* imei;
    // uint8_t* packet;

    void begin(const char* deviceId, const char* imei);
    String createLoginPacket(const char* imei, const char* deviceId);
    String createDeviceDataPacket(float longitude, float latitude, uint64_t timestamp, uint8_t satellites, uint16_t acceleration, uint8_t state, uint16_t battVoltage);
    uint16_t calculateCRC16(uint8_t* data, size_t dataSize);
};

#endif