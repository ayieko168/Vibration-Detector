#include <SoftwareSerial.h>
#include "CodecKT1.h"
#include "TCPComms.h"

const char* deviceId = "gBhmSbJlmIHuRbvgxfRajJTrQSGoZoZqJZDEPNZH";  //Must be 40 Bytes
char* imei = "0356307042441013";                                    //Must be 16 bytes  sprintf(imei, "%016s", imei);
String deviceLoginPacketString = "NULL";
const int BOARD_RESET_PIN = 2;

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

int connectedToInterent;

CodecKT1 codec;
TCPComms tcpcoms;

void setup() {

  digitalWrite(BOARD_RESET_PIN, HIGH);
  pinMode(BOARD_RESET_PIN, OUTPUT); 

  /* ------------- Initialize the serial ports ---------------- */
  // The USB serial
  Serial.begin(9600);

  // The SIM8000 Serial
  bool tcpcoms_started = false;
  while (!tcpcoms_started){
    tcpcoms_started = tcpcoms.begin(10, 11, 12);
    Serial.print("SIM800 Serial Connected: ");
    Serial.println(tcpcoms_started);
    delay(100);
  }

  /* --------- Set the IMEI Number from the device ----------*/
  String newImeiString = tcpcoms.getImeiNumber();
  imei = strdup(newImeiString.c_str());
  Serial.print("IMEI Number set to: ");
  Serial.println(imei);

  /* -------------- Connect to the Internet ----------------*/
  int connectionStatus = -1;
  int connectionRetries = 0;
  while (connectionStatus != 200 && connectionStatus != 202){
    connectionStatus = tcpcoms.connectInternet();
    Serial.print("Internet Connection Status: ");
    Serial.println(connectionStatus); 
    delay(5000);

    connectionRetries ++;
    if (connectionRetries >= 5){
      Serial.println("Resetting the SIM800 and retrying in 10 Seconds...");
      tcpcoms.resetSim800();
      connectionRetries = 0;
      delay(10000);
    }
  }
  


  // Init the comms modile
  // Serial.println(tcpcoms.begin(10, 11, 12));
  // tcpcoms.resetSim800();
  // Serial.println(tcpcoms.getImeiNumber());

  // Serial.println(tcpcoms.connectInternet());
  // Serial.println(tcpcoms.getLocolIP());
  // Serial.println(tcpcoms.sendDataWithResponse(String("NEW MESSAGE FROM DEVICE!!")));

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

  /* ------------ Create the login packet if not created --------------- */
  if (deviceLoginPacketString == "NULL"){
    deviceLoginPacketString = codec.createLoginPacket(imei, deviceId);
    Serial.println("Device Login Packet:");
    Serial.println(deviceLoginPacketString);
  }

  /* --------------- Send the login packet -----------------*/
  String serverResponse = tcpcoms.sendDataWithResponse(deviceLoginPacketString);
  Serial.println("Server respponse:");
  Serial.println(serverResponse);

  /* --------------- Send the data packet -----------------*/
  String serverResponse = tcpcoms.sendDataWithResponse(deviceLoginPacketString);
  Serial.println("Server respponse:");
  Serial.println(serverResponse);

  delay(5000);  // Wait X seconds before sending another packet.
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

