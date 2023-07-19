#include <SoftwareSerial.h>

#define SIM800_TX_PIN 10  //SIM800 TX is connected to Arduino Pin defined here
#define SIM800_RX_PIN 11  //SIM800 RX is connected to Arduino Pin defined here

//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN, SIM800_RX_PIN);

void setup() {

  Serial.begin(9600);

  while (!Serial);
  serialSIM800.begin(9600);
  delay(1000);
  Serial.println("Setup Complete!");
}

void loop() {

  //Read SIM800 output (if available) and print it in Arduino IDE Serial Monitor
  if (serialSIM800.available()) {
    Serial.write(serialSIM800.read());
  }

  //Read Arduino IDE Serial Monitor inputs (if available) and send them to SIM800
  if (Serial.available()) {
    serialSIM800.write(Serial.read());
  }
}