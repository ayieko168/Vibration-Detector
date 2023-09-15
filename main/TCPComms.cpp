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
  sim800->println(F("AT"));
  _readBuffer(500); // Wait for the response
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", the module is not ready.
    // Serial.println(_buffer);
    return 201; // Error code: 201 - SIM800 module not ready
  }

  // Step 2: Check if the SIM card is attached and registered to the network
  sim800->println(F("AT+CGATT?"));
  _readBuffer(200);
  if ((_buffer.indexOf(F("+CGATT: 1"))) == -1) {
    // If the response does not contain "+CGATT: 1", the SIM card is not attached or not registered.
    // Serial.println(_buffer);
    return 211; // Error code: 211 - SIM card not attached or not registered
  }

  // Step 3: Check internet connectivity
  sim800->println(F("AT+CIPSTART=\"TCP\",\"8.8.8.8\",\"53\""));
  _readBuffer(10000);
  if ((_buffer.indexOf(F("CONNECT OK"))) == -1) {
    // If the response does not contain "CONNECT OK", the TCP connection failed.
    // Serial.println(_buffer);
    return 222; // Error code: 222 - Not connected to the internet
  }else{
    // Connection is OK. Close the TCP connection immediately since we only wanted to check connectivity
    sim800->println(F("AT+CIPCLOSE"));
    _readBuffer(500);
    return 202;
  }


  // Step 4: Set the Access Point Name (APN)
  sim800->println(F("AT+CSTT=\"Safaricom\""));
  _readBuffer(500);
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", setting the APN failed.
    // Serial.println(_buffer);
    return 233; // Error code: 233 - Failed to set APN
  }

  // Step 5: Start GPRS (Connect to the internet)
  sim800->println(F("AT+CIICR"));
  _readBuffer(4000);
  if ((_buffer.indexOf(F("OK"))) == -1) {
    // If the response does not contain "OK", GPRS connection failed.
    // Serial.println(_buffer);
    return 244; // Error code: 244 - Failed to start GPRS
  }

  // Step 6: Get Local IP
  sim800->println(F("AT+CIFSR"));
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
  sim800->println(F("AT+CIPSTATUS"));
  _readBuffer(500);
  bool tcpConnected = _buffer.indexOf(F("STATE: CONNECT OK")) != -1;

  if (!tcpConnected) {
    // TCP connection is not active, establish new connection
    sim800->println(F("AT+CIPSTART=\"TCP\",\"151.80.209.133\",\"6500\""));
    delay(2000);
    // _readBuffer(10000);
    // Serial.println(_buffer);
    // if (_buffer.indexOf(F("CONNECT OK")) == -1) {
    //   response = F("Error: Failed TCP connect");

    //   // Close the TCP connection if it was not already open
    //   if (!tcpConnected) {
    //     sim800->println(F("AT+CIPCLOSE"));
    //     _readBuffer(200);
    //   }
    //   return response;
    // }
  }

  // Sending data to the server
  sim800->print(F("AT+CIPSEND="));
  sim800->println(payload.length());
  delay(100); // Wait for the ">"
  sim800->print(payload); // Send the payload
  delay(100);
  sim800->write(26); // Send Ctrl+Z (ASCII code 26)

  // Wait for the response from the server
  _readBuffer(1000);

  // unsigned long timeout = millis() + 5000; // Set a timeout of 5 seconds
  // _buffer = ""; // Clear the buffer

  // bool responseStarted = false;
  // while (timeout > millis()) {
  //   if (sim800->available()) {
  //     char c = sim800->read();
  //     _buffer += c;

  //     // Check if the server response has started
  //     if (!responseStarted) {
  //       int sendOkIndex = _buffer.indexOf(F("SEND OK"));
  //       if (sendOkIndex != -1) {
  //         responseStarted = true;
  //         _buffer = _buffer.substring(sendOkIndex + 8); // Skip "SEND OK" and the newline characters
  //       }
  //     }
  //   }
  // }

  // // Check if the server response has been fully received
  // if (!responseStarted) {
  //   response = F("Error: Bad Server response or timeout");
  // } else {
  //   // Remove leading newline characters, if any
  //   _buffer.trim();

  //   // Check if "CLOSED" is present at the end of the response and remove it
  //   int closedIndex = _buffer.indexOf(F("CLOSED"));
  //   if (closedIndex != -1 && closedIndex == (_buffer.length() - 6)) {
  //     _buffer = _buffer.substring(0, closedIndex);
  //     _buffer.trim();
  //   }

  //   // Extract the server response (if available)
  //   response = _buffer;
  // }

  // // Close the TCP connection if it was not already open
  // sim800->println(F("AT+CIPCLOSE"));
  // _readBuffer(200);

  response = _buffer;
  return response;
}

String TCPComms::getLocolIP() {
  // Get Local IP
  sim800->println(F("AT+CIFSR"));
  _readBuffer(400);
  if ((_buffer.indexOf(F("."))) != -1) {

    // Serial.println("LOCAL IP: ");
    // Serial.println(_buffer);
    String ret_buff = _buffer.substring(String(F("AT+CIFSR")).length());
    ret_buff.trim();
    return ret_buff;  // Success

  } else {
    return F("NONE");
  }
}

String TCPComms::getImeiNumber() {
  // Get IMEI Number
  sim800->println(F("AT+GSN"));
  _readBuffer(400);
  if ((_buffer.indexOf(F("OK"))) != -1) {
    int imeiStart = _buffer.indexOf('\n') + 1; // Find the start of the IMEI number
    int imeiEnd = _buffer.indexOf('\r', imeiStart); // Find the end of the IMEI number (before the carriage return)
    
    if (imeiEnd - imeiStart == 15) {
      String imei = _buffer.substring(imeiStart, imeiEnd); // Extract the IMEI number
      imei = "0" + imei; // Pad the IMEI to 16 characters with leading zero
      return imei;
    } else {
      return F("NONE2"); // Invalid IMEI length
    }
  } else {
    return F("NONE1"); // Error in AT command response
  }
}

int TCPComms::setCurrentTime() {

  // Check if network time updating is disabled (CLTS mode is 0)
  sim800->println(F("AT+CLTS?"));
  _readBuffer(200);
  if (_buffer.indexOf(F("+CLTS: 0")) != -1) {
    
    // Set network time updating to enabled (CLTS mode to 1)
    sim800->println(F("AT+CLTS=1;&W"));
    delay(100);
    
    // Restart the modem
    sim800->println(F("AT+CFUN=1,1"));
    delay(4000); // Give some time for the modem to restart
    
    // Confirm whether the modem is in CLTS mode
    sim800->println(F("AT+CLTS?"));
    _readBuffer(5000);
    if (_buffer.indexOf(F("+CLTS: 1")) != -1) {
      // Serial.println("CLTS mode activated.");
      
      // Get the current time
      sim800->println(F("AT+CCLK?"));
      _readBuffer(2000);
      if (_buffer.indexOf(F("CCLK:")) != -1) {
        
        // Extract and validate the year (yy)
        String response = _buffer.substring(_buffer.indexOf(F("CCLK:")) + 7);
        int year = response.substring(0, 2).toInt();
        if (year >= 23) {
          Serial.println("Current time: " + response);
          return 201;
        } else {
          Serial.println(F("Invalid year."));
          return 233;
        }
      } else {
        // Serial.println("Failed to get current time.");
        return 222;
      }
    } else {
      // Serial.println("Failed to activate CLTS mode.");
      return 211;
    }
  } else {
    // Serial.println("CLTS mode is already activated or unknown response.");
    return 200;
  }
}

unsigned long TCPComms::getTimestamp() {

  String response = "";
  // Get the current time from the SIM800 module
  sim800->println(F("AT+CCLK?"));
  _readBuffer(1000);

  // Check if the response contains the expected format
  // Serial.println(_buffer);
  response += _buffer;
  if (response.indexOf(F("+CCLK: \"")) != -1 && response.indexOf(F("OK")) != -1) {
    response = response.substring(response.indexOf(F("+CCLK: \"")) + 8, response.indexOf(F("OK")) - 2);
    response.trim();

    // Extract the date and time components from the response
    int year = response.substring(0, 2).toInt();
    int month = response.substring(3, 5).toInt();
    int day = response.substring(6, 8).toInt();

    // Extracting time components
    int hour = response.substring(9, 11).toInt();
    int minute = response.substring(12, 14).toInt();
    int second = response.substring(15, 17).toInt();

    // Correcting for negative minute values
    if (minute < 0) {
      minute += 60;
      hour--;
    }

    // Correcting for negative hour values
    if (hour < 0) {
      hour += 24;
      day--;
    }

    // Correcting for negative day values
    if (day <= 0) {
      // Assume the previous month had 31 days (this could be improved for other cases)
      day += 31;
      month--;
    }

    // Correcting for negative month values
    if (month <= 0) {
      month += 12;
      year--;
    }

    // Verify the year is valid
    // if (year < 23) {

    //   Serial.print("Invalid year: ");
    //   Serial.println(year);
    //   return 99;
    // } 

    // Correcting for 2-digit year values
    year += 2000;

    // Print the extracted and adjusted time
    Serial.print(F("Extracted Time: "));
    Serial.print(year);
    Serial.print(F("/"));
    if (month < 10) Serial.print("0");
    Serial.print(month);
    Serial.print(F("/"));
    if (day < 10) Serial.print("0");
    Serial.print(day);
    Serial.print(F(","));
    if (hour < 10) Serial.print("0");
    Serial.print(hour);
    Serial.print(F(":"));
    if (minute < 10) Serial.print("0");
    Serial.print(minute);
    Serial.print(F(":"));
    if (second < 10) Serial.print("0");
    Serial.println(second);

    // Days per month in a non-leap year
    const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Calculate total days from 1970 to the given year
    unsigned long totalDays = 0;
    for (int y = 1970; y < year; y++) {
        totalDays += 365;
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) {
            // Leap year has an extra day
            totalDays++;
        }
    }

    // Calculate total days in the current year up to the given month
    for (int m = 1; m < month; m++) {
        totalDays += daysInMonth[m - 1];
        if (m == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
            // February in a leap year has 29 days
            totalDays++;
        }
    }

    // Add the day of the month
    totalDays += day - 1;

    // Calculate total seconds
    unsigned long totalSeconds = totalDays * 86400UL + hour * 3600UL + minute * 60UL + second;

    // Adjust for leap years in the current year
    if ((month > 2) && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        totalSeconds += 86400UL; // Add one extra day for leap year
    }

    // Serial.println(totalSeconds);
    return totalSeconds;

  } else {
    Serial.println(F("Error: Response From CLK CMD: "));
    Serial.println(response);
    return 99; // Error code: Unexpected response format
  }
}

void TCPComms::_readBuffer(uint32_t timeout = 5000) {
  _buffer = "";
  _buffer.reserve(20);

  uint32_t start_time = millis();
  while (millis() - start_time < timeout) {
    if (sim800->available()) {
      char c = sim800->read();
      _buffer += c;
    }
  }
}

// void TCPComms::_readBuffer(uint32_t timeout = 5000) {
// // Reads the serial buffer only when there is data flowing, else exit.
//   _buffer = "";

//   uint32_t start_time = millis();
//   while ((millis() - start_time < timeout) && sim800->available()) {
//     char c = sim800->read();
//     _buffer += c;
//   }
// }

