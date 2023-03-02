#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

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

Adafruit_MPU6050 mpu;

void blink_indicator(int times, int delay_time = 500){
  for(int i = 1; i <= times; i++){
    digitalWrite(ready_led_pin, HIGH);
    delay(delay_time);
    digitalWrite(ready_led_pin, LOW);
    delay(delay_time);
  }
}

double my_map(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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

void setup(void) {
  Serial.begin(115200);

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

  blink_indicator(3);
  // set accelerometer and gyro range.
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);

  

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

        //Send Notification
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

        //Send Notification
    }
     
    // Set state
    state = LOW;
    digitalWrite(activity_led_pin, state); // Update the LED immediately when condition is false
    start_time = 0; // Reset start_time to zero

    
  }

}
