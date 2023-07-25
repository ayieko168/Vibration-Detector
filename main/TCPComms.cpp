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
    // Serial.println(_buffer);
    return 201; // Error code: 201 - SIM800 module not ready
  }

  // Step 2: Check if the SIM card is attached and registered to the network
  sim800->println("AT+CGATT?");
  _readBuffer(200);
  if ((_buffer.indexOf(F("+CGATT: 1"))) == -1) {
    // If the response does not contain "+CGATT: 1", the SIM card is not attached or not registered.
    // Serial.println(_buffer);
    return 211; // Error code: 211 - SIM card not attached or not registered
  }

  // Step 3: Check internet connectivity
  sim800->println("AT+CIPSTART=\"TCP\",\"8.8.8.8\",\"53\"");
  _readBuffer(5000);
  if ((_buffer.indexOf(F("CONNECT OK"))) == -1) {
    // If the response does not contain "CONNECT OK", the TCP connection failed.
    // Serial.println(_buffer);
    return 222; // Error code: 222 - Not connected to the internet
  }else{
    // Connection is OK. Close the TCP connection immediately since we only wanted to check connectivity
    sim800->println("AT+CIPCLOSE");
    _readBuffer(500);
    return 202;
  }


  // Step 4: Set the Access Point Name (APN)
  sim800->println("AT+CSTT=\"Safaricom\"");
  _readBuffer(500);
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", setting the APN failed.
    // Serial.println(_buffer);
    return 233; // Error code: 233 - Failed to set APN
  }

  // Step 5: Start GPRS (Connect to the internet)
  sim800->println("AT+CIICR");
  _readBuffer(4000);
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", GPRS connection failed.
    // Serial.println(_buffer);
    return 244; // Error code: 244 - Failed to start GPRS
  }

  // Step 6: Get Local IP
  sim800->println("AT+CIFSR");
  _readBuffer(200);
  if ((_buffer.indexOf(F("."))) == -1) {
    // If the response does not contain ".", getting the local IP failed.
    // Serial.println(_buffer);
    return 255; // Error code: 255 - Failed to get local IP
  }

  // If all steps are successful, return success status.
  // Serial.println(_buffer);
  return 200; // Success - Connected to the internet
}

String TCPComms::sendDataWithResponse(const String& payload) {
  String response;

  // Check if TCP connection is already active
  sim800->println("AT+CIPSTATUS");
  _readBuffer(500);
  bool tcpConnected = _buffer.indexOf(F("STATE: CONNECT OK")) != -1;

  if (!tcpConnected) {
    // TCP connection is not active, establish the connection
    sim800->println("AT+CIPSTART=\"TCP\",\"151.80.209.133\",\"6500\"");
    _readBuffer(5000);
    if (_buffer.indexOf(F("CONNECT OK")) == -1) {
      response = "Error: Failed to establish TCP connection";
      return response;
    }
  }

  // Sending data to the server
  sim800->print("AT+CIPSEND=");
  sim800->println(payload.length());
  delay(100); // Wait for the ">"
  sim800->print(payload); // Send the payload
  sim800->write(26); // Send Ctrl+Z (ASCII code 26)

  // Wait for the response from the server
  unsigned long timeout = millis() + 5000; // Set a timeout of 5 seconds
  _buffer = ""; // Clear the buffer

  bool responseStarted = false;
  while (timeout > millis()) {
    if (sim800->available()) {
      char c = sim800->read();
      _buffer += c;

      // Check if the server response has started
      if (!responseStarted) {
        int sendOkIndex = _buffer.indexOf(F("SEND OK"));
        if (sendOkIndex != -1) {
          responseStarted = true;
          _buffer = _buffer.substring(sendOkIndex + 8); // Skip "SEND OK" and the newline characters
        }
      }
    }
  }

  // Check if the server response has been fully received
  if (!responseStarted) {
    response = "Error: Server response not fully received or timeout";
  } else {
    // Remove leading newline characters, if any
    _buffer.trim();

    // Check if "CLOSED" is present at the end of the response and remove it
    int closedIndex = _buffer.indexOf(F("CLOSED"));
    if (closedIndex != -1 && closedIndex == (_buffer.length() - 6)) {
      _buffer = _buffer.substring(0, closedIndex);
      _buffer.trim();
    }

    // Extract the server response (if available)
    response = _buffer;
  }

  // Close the TCP connection if it was not already open
  if (!tcpConnected) {
    sim800->println("AT+CIPCLOSE");
    _readBuffer(200);
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

void TCPComms::_readBuffer(uint32_t timeout = 5000) {
  _buffer = "";
  uint64_t timeOld = millis();
  while (!sim800->available() && !(millis() > timeOld + timeout)) { ; }
  if (sim800->available()) { _buffer = sim800->readString(); }
}