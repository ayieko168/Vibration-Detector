#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define ready_led_pin 8
#define activity_led_pin 9
#define state_change_thresh 3.0
#define state_time_thresh 5000

double initial_read;
unsigned long start_time;
long previousMillis_h = 0;
long previousMillis_l = 0;
double state = LOW;

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
  
  //Check for on:
  double new_reading = get_reading();
  Serial.print(new_reading);
  Serial.print(" State: ");
  Serial.println(state);

  
  if(abs(initial_read - new_reading) > state_change_thresh){

    unsigned long currentMillis_h = millis(); 
    if(abs(currentMillis_h - previousMillis_h) > state_time_thresh){
      
      Serial.print(abs(currentMillis_h - previousMillis_h));
      Serial.println(" THRESHOLD AND TIMER!!!!");

      // Set state
      state = HIGH;
      
      previousMillis_h = currentMillis_h; 
    }
    
    Serial.println("THRESHOLD!!!!");
  } else{
     
      // Set state
      state = LOW;
      
  }

  
  digitalWrite(activity_led_pin, state);
  
}
