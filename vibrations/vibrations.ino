#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#define ready_led_pin 8
#define activity_led_pin 9
#define pot_pin A6
#define state_time_thresh 3000

double state_change_thresh = 3.0;
double initial_read;
unsigned long start_time;
long previousMillis_h = 0;
long previousMillis_l = 0;
int state = LOW;


String apn = "telkom";                    //APN
String apn_u = "";                     //APN-Username
String apn_p = "";                     //APN-Password
String url = "http://testserver.aeq-web.com/sim800_test/sim800.php";  //URL of Server

Adafruit_MPU6050 mpu;
SoftwareSerial SWserial(7, 6); // RX, TX

void blink_indicator(int times, int delay_time = 500){
  for(int i = 1; i <= times; i++){
    digitalWrite(ready_led_pin, HIGH);
    delay(delay_time);
    digitalWrite(ready_led_pin, LOW);
    delay(delay_time);
  }
}
   
double get_reading(){

  double gf_sum = 0.0;
  for(int i = 1; i <= 25; i++){
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    double x = g.gyro.x;
    double y = g.gyro.y;
    double z = g.gyro.z;
    
    double gf = sqrt(sq(x) + sq(y) + sq(z));
    
    gf_sum += gf;
    delay(5);
  }

  return gf_sum;
}

void gsm_http_post( String postdata) {
  Serial.println(" --- Start GPRS & HTTP --- ");
  gsm_send_serial("AT+SAPBR=1,1");
  gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url);
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  gsm_send_serial("AT+HTTPDATA=192,5000");
  gsm_send_serial(postdata);
  gsm_send_serial("AT+HTTPACTION=1");
  gsm_send_serial("AT+HTTPREAD");
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
}

void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  gsm_send_serial("AT+SAPBR=3,1,Contype,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}

void gsm_send_serial(String command) {
  Serial.println("Send ->: " + command);
  SWserial.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (SWserial.available()) {
      Serial.write(SWserial.read());
    }
  }
  Serial.println();
}

void setup(void) {
  Serial.begin(115200);
  SWserial.begin(9600);

  //Setup LED Pins
  pinMode(ready_led_pin, OUTPUT);
  pinMode(activity_led_pin, OUTPUT);
  digitalWrite(ready_led_pin, LOW);
  digitalWrite(activity_led_pin, LOW);
  
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    blink_indicator(2);
    while (1) {
      delay(10);
    }
  }

  while (SWserial.available()) {
    Serial.write(SWserial.read());
  }
  

  blink_indicator(3);
  // set accelerometer and gyro range.
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);

  gsm_config_gprs();

  delay(100);

  // Set the initilal read.
  initial_read = get_reading();
}

void loop() {

  int potVal = analogRead(pot_pin);
  double new_reading = get_reading();
  state_change_thresh = map(potVal, 0, 750, 1, 20);
  
  Serial.print("Pot: ");
  Serial.print(state_change_thresh);
  Serial.print(" Thresh Delta: ");
  Serial.print(abs(initial_read - new_reading));
  Serial.print(" New Reading: ");
  Serial.print(new_reading);
  Serial.print(" State: ");
  Serial.println(state);

  static unsigned long start_time = 0; // Declare start_time as static
  unsigned long currentMillis = millis();

  //Check for on:
  if (abs(initial_read - new_reading) > state_change_thresh) {

    if (!start_time) { // If start_time is zero, assign the current time
      start_time = currentMillis;
    }
    
    if (currentMillis - start_time >= state_time_thresh) { // Check if the elapsed time exceeds the threshold

      if(state != HIGH){
        Serial.println("THRESHOLD AND TIMER!!! Sending On Notification");
        Serial.println();
        
        //Send Notification
        //gsm_http_post("param=TestFromMySim800");
        gsm_send_serial("AT+CCLK?");
      }
      

      // Set state
      state = HIGH;
      digitalWrite(activity_led_pin, state); // Update the LED immediately after the threshold is reached

      start_time = 0; // Reset start_time to zero
    }
    
    Serial.println("THRESHOLD!!!!");
  } 
  else {

    if(state != LOW){
        Serial.println("NO THRESHOLD AND TIMER!!! Sending Off Notification");
        Serial.println();
        
        //Send Notification
        gsm_http_post("param=TestFromMySim800");
    }
     
    // Set state
    state = LOW;
    digitalWrite(activity_led_pin, state); // Update the LED immediately when condition is false
    start_time = 0; // Reset start_time to zero

    
  }

}
