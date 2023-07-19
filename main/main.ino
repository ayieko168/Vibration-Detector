#include <SoftwareSerial.h>
#include "CodecKT1.h"
#include "TCPComms.h"

const char* deviceId = "gBhmSbJlmIHuRbvgxfRajJTrQSGoZoZqJZDEPNZH";  //Must be 40 Bytes
char* imei = "0356307042441013";                                    //Must be 16 bytes  sprintf(imei, "%016s", imei);
String packet = "79463803465475596D797443446153456F634E58485150444F4875466955795178576C5859744F6341676577746665564C7176585068514E47724B45B3D86497";

float longitude = -1.349856;
float latitude = 32.455678;
uint64_t timestamp = 1689359519;
uint8_t satellites = 10;
uint16_t acceleration = 512;
uint8_t state = 1;
uint16_t battVoltage = 3600;

// TCP Variables
String apn = "Safaricom";
String server_ip = "151.80.209.133";
String server_port = "6500";

CodecKT1 codec;
TCPComms tcpcoms;

void setup() {

  Serial.begin(9600);

  // Init the comms modile
  Serial.println(tcpcoms.begin(10, 11, 12));
  tcpcoms.resetSim800();
  delay(5000);
  Serial.println(tcpcoms.getImeiNumber());

  Serial.println(tcpcoms.connectInternet());
  Serial.println(tcpcoms.getLocolIP());
  Serial.println(tcpcoms.sendDataWithResponse(String("NEW MESSAGE FROM DEVICE!!")));

  // String deviceDataPacketString = codec.createDeviceDataPacket(longitude, latitude, timestamp, satellites, acceleration, state, battVoltage);
  // Serial.println("Device Data Packet:");
  // Serial.println(deviceDataPacketString);

  // String deviceLoginPacketString = codec.createLoginPacket(imei, deviceId);
  // Serial.println("Device Login Packet:");
  // Serial.println(deviceLoginPacketString);

  // String serverAcknowledgment = codec.verifyAcknowledgmentPacket(packet, deviceId);
  // Serial.println("Server Acknowledgment:");
  // Serial.println(serverAcknowledgment);

}

void loop() {

}


void checkLoginDataLenths(){
  /*
  A utility that can be used to check the length of the device_id and device_imei values.
  This function should be run every time a the begining of the loop.
  */

  size_t deviceIdLength = strlen(deviceId);
  size_t imeiLength = strlen(imei);

  if(deviceIdLength < 40 || imeiLength < 16){
    Serial.print(F("[CRITICAL] One of the Login Data parts has a length less than needed; "));
    Serial.print("Devive ID len: ");
    Serial.print(deviceIdLength);
    Serial.print(" IMEI len: ");
    Serial.println(imeiLength);
  }
  
}

