#include "TCPComms.h"

int tx_pin;
int rx_pin;
int reset_pin;
SoftwareSerial* sim800 = NULL;

bool TCPComms::begin(int _tx_pin, int _rx_pin, int _reset_pin) {

  if (sim800 == NULL) {
    // Check if the SoftwareSerial is not already created

    tx_pin = _tx_pin;
    rx_pin = _rx_pin;
    reset_pin = _reset_pin;

    sim800 = new SoftwareSerial(tx_pin, rx_pin);  // Create a new SoftwareSerial instance
    sim800->begin(9600);                          // Initialize the SoftwareSerial


    // Test the serial
    sim800->println(F("AT"));
    _readBuffer(300);

    // Read returned
    if ((_buffer.indexOf(F("OK"))) != -1) {
      return true;
    } else {
      return false;
    }
  } else {
    return true;
  }
}

int TCPComms::connectInternet() {

  // Attach GPRS service: <Blinks indicate network connection state>
  sim800->println("AT+CGATT?");
  _readBuffer(500);
  if ((_buffer.indexOf(F("+CGATT: 1"))) != -1) {

    // // Check if connected to INTERNET
    // sim800->println("AT+CIPSTATUS");
    // _readBuffer(500);
    // if((_buffer.indexOf(F("STATE: IP STATUS"))) != -1){

    // Set the APN
    sim800->println("AT+CSTT=\"Safaricom\"");
    _readBuffer(500);
    if ((_buffer.indexOf(F("OK"))) != -1) {

      // Start GPRS (Connect to internet)
      sim800->println("AT+CIICR");
      _readBuffer(3000);
      if ((_buffer.indexOf(F("OK"))) != -1) {

        // Get Local IP
        sim800->println("AT+CIFSR");
        _readBuffer(400);
        if ((_buffer.indexOf(F("."))) != -1) {

          Serial.println("LOCAL IP: ");
          // Serial.println(_buffer);

          return 200;  // Success

        } else {
          return 244;
        }

      } else {
        return 233;
      }

    } else {
      return 222;
    }
    // }else {
    //   return 201; // Already connected
    // }

  } else {
    return 211;
  }
}

String TCPComms::sendLoginHandShake() {
  String response;

  // Check if TCP connection is already active
  sim800->println("AT+CIPSTATUS");
  _readBuffer(1000);
  if (_buffer.indexOf(F("STATE: CONNECT OK")) != -1) {
    // TCP connection is already active
    sim800->println("AT+CIPSEND");
    delay(100); // Wait for the ">"
    sim800->println("MESSAGE FROM DEVICE!");
    sim800->write(26); // Send Ctrl+Z (ASCII code 26)
    _readBuffer(500);
    if (_buffer.indexOf(F("SEND OK")) != -1) {
      response = _buffer;
    } else {
      response = "Error: Failed to send data";
    }
  } else {
    // TCP connection is not active, establish the connection
    sim800->println("AT+CIPSTART=\"TCP\",\"151.80.209.133\",\"6500\"");
    _readBuffer(5000);
    if (_buffer.indexOf(F("CONNECT OK")) != -1) {
      sim800->println("AT+CIPSEND");
      delay(100); // Wait for the ">"
      sim800->println("MESSAGE FROM DEVICE!");
      sim800->write(26); // Send Ctrl+Z (ASCII code 26)
      _readBuffer(500);
      if (_buffer.indexOf(F("SEND OK")) != -1) {
        response = _buffer;
      } else {
        response = "Error: Failed to send data";
      }
    } else {
      response = "Error: Failed to establish TCP connection";
    }
  }

  return response;
}

bool TCPComms::sendDeviceData() {
  
}

String TCPComms::getLocolIP() {
  // Get Local IP
  sim800->println("AT+CIFSR");
  _readBuffer(400);
  if ((_buffer.indexOf(F("."))) != -1) {

    // Serial.println("LOCAL IP: ");
    // Serial.println(_buffer);
    String ret_buff = _buffer.substring(String("AT+CIFSR").length());
    ret_buff.trim();
    return ret_buff;  // Success

  } else {
    return "NONE";
  }
}

String TCPComms::getImeiNumber() {
  // Get Local IP
  sim800->println("AT+GSN");
  _readBuffer(400);
  if ((_buffer.indexOf(F("OK"))) != -1) {

    String ret_buff = _buffer.substring(String("AT+CIFSR").length());
    ret_buff.trim();
    return ret_buff;  // Success

  } else {
    return "NONE";
  }
}

void TCPComms::testVars() {
  Serial.println(tx_pin);
  Serial.println(rx_pin);
  Serial.println(reset_pin);

  sim800->println("AT");
  _readBuffer();

  Serial.println(_buffer);
}

void TCPComms::_readBuffer(uint32_t timeout = 2000) {
  _buffer = "";
  uint64_t timeOld = millis();
  while (!sim800->available() && !(millis() > timeOld + timeout)) { ; }
  if (sim800->available()) { _buffer = sim800->readString(); }
}