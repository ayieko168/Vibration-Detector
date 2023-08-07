#include <Wire.h>
#include <SoftwareSerial.h>

char* imei = "NULL0";
String deviceLoginPacketString = "NULL";
const int BOARD_RESET_PIN = 2;
String serverResponse;
int connectedToInterent;

