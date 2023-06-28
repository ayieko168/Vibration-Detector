#include <SoftwareSerial.h>

#define TIME_OUT_READ_SERIAL 300

String _buffer;

const char* imei = "12345678";
const char* deviceId = "ABCDEF1234567890";
float longitude = 12.3456;
float latitude = 34.5678;
uint64_t timestamp = 1624907412;
uint8_t satellites = 10;
uint16_t acceleration = 512;
uint8_t state = 1;
uint16_t battVoltage = 3600;

// Assuming a buffer of size 100 to hold the packets
uint8_t loginPacket[100];
uint8_t deviceDataPacket[100];

// TCP Variables
String apn = "Safaricom";
String server_ip = "151.80.209.133";
String server_port = "6500";


SoftwareSerial sim800(3, 4); // RX, TX


void _readSerial() {
	_buffer = "";
	uint64_t timeOld = millis();
	// Wait a while...
	while (!sim800.available() && !(millis() > timeOld + TIME_OUT_READ_SERIAL)) { ; }
	// When you leave without waiting or when the time is up, read the serial if you have any, otherwise go out...
	if(sim800.available()) { 
    _buffer = sim800.readString(); 
    Serial.println(_buffer);
  }
}

void send_data( String _payload) {
  
  // set apn command
  String apn_command = "AT+CSTT=\"" + apn + "\"";
  String tcp_command = "AT+CIPSTART=\"TCP\",\"" + server_ip + "\",\"" + server_port + "\"";

  // Send the commands
  sim800.println("AT");
  _readSerial();
  sim800.println(apn_command);
  _readSerial();
  sim800.println("AT+CIICR");
  _readSerial();
  sim800.println("AT+CIFSR");
  _readSerial();
  sim800.println(tcp_command);
  _readSerial();
  sim800.println("AT+CIPSEND");
  _readSerial();
  sim800.println(_payload);
  _readSerial();
  sim800.write(0x1a);
  _readSerial();
  
}

// Function to create the login packet
void createLoginPacket(uint8_t* packet, const char* imei, const char* deviceId) {
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
  uint16_t crc = calculateCRC16(packet - packetLength, packetLength);
  *packet++ = crc >> 8;
  *packet++ = crc & 0xFF;

  // Stop bit
  *packet++ = 0xaa;
  *packet++ = 0xaa;
}

// Function to create the device data packet
void createDeviceDataPacket(uint8_t* packet, float longitude, float latitude, uint64_t timestamp, uint8_t satellites, uint16_t acceleration, uint8_t state, uint16_t battVoltage) {
  // Start bit
  *packet++ = 0xee;
  *packet++ = 0xee;

  // Packet length
  *packet++ = 0x17;

  // Protocol number
  *packet++ = 0x02;

  // Packet data
  memcpy(packet, &longitude, sizeof(longitude));
  packet += sizeof(longitude);
  memcpy(packet, &latitude, sizeof(latitude));
  packet += sizeof(latitude);
  memcpy(packet, &timestamp, sizeof(timestamp));
  packet += sizeof(timestamp);
  *packet++ = satellites;
  memcpy(packet, &acceleration, sizeof(acceleration));
  packet += sizeof(acceleration);
  *packet++ = state;
  memcpy(packet, &battVoltage, sizeof(battVoltage));
  packet += sizeof(battVoltage);

  // Error Check
  uint16_t crc = calculateCRC16(packet - 0x17, 0x17);
  *packet++ = crc >> 8;
  *packet++ = crc & 0xFF;

  // Stop bit
  *packet++ = 0xaa;
  *packet++ = 0xaa;
}

void setup() {
  Serial.begin(115200);
  sim800.begin(9600);

  send_data("hello world!");
}

void loop() {
  // put your main code here, to run repeatedly:

}
