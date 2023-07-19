#ifndef TCPCOMMS_H
#define TCPCOMMS_H

#include <Arduino.h>
#include <SoftwareSerial.h>

#define TIME_OUT_READ_SERIAL	5000

extern int tx_pin;
extern int rx_pin;
extern int reset_pin;
extern SoftwareSerial* sim800;

class TCPComms {

  protected:
    String _buffer;
    void _readBuffer(uint32_t timeout = 2000);

  public:
    bool begin(int tx_pin, int rx_pin, int reset_pin);
    void resetSim800();
    int connectInternet();
    String sendDataWithResponse(const String& payload);
    bool sendDeviceData();

    String getLocolIP();
    String getImeiNumber();
    void testVars();
};

#endif