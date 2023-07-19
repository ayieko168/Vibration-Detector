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

    // Set the reset pin and ensure it is always HIGH
    pinMode(reset_pin, OUTPUT);
    digitalWrite(reset_pin, HIGH);

    // Reset the SIM800 at first start
    // digitalWrite(reset_pin, LOW);
    // delay(100);
    // digitalWrite(reset_pin, HIGH);

    // Create the software serial
    sim800 = new SoftwareSerial(tx_pin, rx_pin);  // Create a new SoftwareSerial instance
    sim800->begin(9600);                          // Initialize the SoftwareSerial

    delay(500);
    
    // Test the serial
    sim800->println(F("AT"));
    _readBuffer(300);
    if ((_buffer.indexOf(F("OK"))) != -1) {
      return true;
    } 
    else {

      delete sim800;
      sim800 = NULL;

      return false;
    }
  } 
  else {
    return true;
  }
}

void TCPComms::resetSim800() {
  digitalWrite(reset_pin, LOW);
  delay(100);
  digitalWrite(reset_pin, HIGH);

  delay(2000);
}

int TCPComms::connectInternet() {

  // Step 1: Check if the SIM800 module is ready
  sim800->println("AT");
  _readBuffer(500); // Wait for the response
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", the module is not ready.
    Serial.println(_buffer);
    return 201; // Error code: 201 - SIM800 module not ready
  }

  // Step 2: Check if the SIM card is attached and registered to the network
  sim800->println("AT+CGATT?");
  _readBuffer(200);
  if ((_buffer.indexOf(F("+CGATT: 1"))) == -1) {
    // If the response does not contain "+CGATT: 1", the SIM card is not attached or not registered.
    Serial.println(_buffer);
    return 211; // Error code: 211 - SIM card not attached or not registered
  }

  // // Step 3: Check if already connected to the internet
  // sim800->println("AT+CIPSTATUS");
  // _readBuffer(200);
  // if ((_buffer.indexOf(F("STATE: IP STATUS"))) != -1) {
  //   // If the response contains "STATE: IP STATUS", it means the module is already connected to the internet.
  //   return 209; // Error code: 299 - Already connected to the internet
  // }

  // Step 4: Set the Access Point Name (APN)
  sim800->println("AT+CSTT=\"Safaricom\"");
  _readBuffer(500);
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", setting the APN failed.
    Serial.println(_buffer);
    return 222; // Error code: 222 - Failed to set APN
  }

  // Step 5: Start GPRS (Connect to the internet)
  sim800->println("AT+CIICR");
  _readBuffer(6000);
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", GPRS connection failed.
    Serial.println(_buffer);
    return 233; // Error code: 233 - Failed to start GPRS
  }

  // Step 6: Get Local IP
  sim800->println("AT+CIFSR");
  _readBuffer(200);
  if ((_buffer.indexOf(F("."))) == -1) {
    // If the response does not contain ".", getting the local IP failed.
    Serial.println(_buffer);
    return 244; // Error code: 244 - Failed to get local IP
  }

  // If all steps are successful, return success status.
  Serial.println(_buffer);
  return 200; // Success - Connected to the internet
}

String TCPComms::sendDataWithResponse(const String& payload) {
  String response;

  // Check if TCP connection is already active
  sim800->println("AT+CIPSTATUS");
  _readBuffer(500);
  if (_buffer.indexOf(F("STATE: CONNECT OK")) != -1) {
    // TCP connection is already active
    sim800->println("AT+CIPSEND");
    delay(100); // Wait for the ">"
    sim800->println(payload); // Send the payload
    sim800->write(26); // Send Ctrl+Z (ASCII code 26)
    _readBuffer(500);
    if (_buffer.indexOf(F("SEND OK")) != -1) {
      String serverResponse = _buffer.substring(_buffer.indexOf("SEND OK") + 8);
      serverResponse.trim();
      response = serverResponse;
    } else {
      response = "Error: Failed to send data";
    }

    // Close the TCP connection
    sim800->println("AT+CIPCLOSE");
    _readBuffer(200);

  } else {
    // TCP connection is not active, establish the connection
    sim800->println("AT+CIPSTART=\"TCP\",\"151.80.209.133\",\"6500\"");
    _readBuffer(5000);
    if (_buffer.indexOf(F("CONNECT OK")) != -1) {
      
      sim800->println("AT+CIPSEND");
      delay(100); // Wait for the ">"
      sim800->println(payload); // Send the payload
      sim800->write(26); // Send Ctrl+Z (ASCII code 26)
      _readBuffer(500);
      if (_buffer.indexOf(F("SEND OK")) != -1) {
        String serverResponse = _buffer.substring(_buffer.indexOf("SEND OK") + 8);
        serverResponse.trim();
        response = serverResponse;
      } else {
        response = "Error: Failed to send data";
      }

      // Close the TCP connection
      sim800->println("AT+CIPCLOSE");
      _readBuffer(200);

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
  // Get IMEI Number
  sim800->println("AT+GSN");
  _readBuffer(400);
  if ((_buffer.indexOf(F("OK"))) != -1) {
    int imeiStart = _buffer.indexOf('\n') + 1; // Find the start of the IMEI number
    int imeiEnd = _buffer.indexOf('\r', imeiStart); // Find the end of the IMEI number (before the carriage return)
    
    if (imeiEnd - imeiStart == 15) {
      String imei = _buffer.substring(imeiStart, imeiEnd); // Extract the IMEI number
      imei = "0" + imei; // Pad the IMEI to 16 characters with leading zero
      return imei;
    } else {
      return "NONE2"; // Invalid IMEI length
    }
  } else {
    return "NONE1"; // Error in AT command response
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