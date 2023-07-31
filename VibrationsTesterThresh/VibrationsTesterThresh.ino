#include <Wire.h>
#include <MPU6050.h>

// Initialize the MPU6050 sensor
MPU6050 mpu;

uint8_t MPU6050_GYRO_FS_250 = 0;
uint8_t MPU6050_GYRO_FS_500 = 1;
uint8_t MPU6050_GYRO_FS_1000 = 2;
uint8_t MPU6050_GYRO_FS_2000 = 3;

float previousReading = 0.0;
float absoluteDisplasement = 0.0;
float gyroscopeMagnitude = 0.0;

float WORKING_THRESHOLD_DISP = 50.0;  // Amplitude
float IDLE_THRESHOLD_DISP = 5.0;
float OFF_THRESHOLD_DISP = 0.0;

int WORKING_THRESHOLD_PERIOD = 5000;  // In MilliSeconds
int IDLE_THRESHOLD_PERIOD = 5000;
int OFF_THRESHOLD_PERIOD = 60000;

unsigned long currentMillis_w;
unsigned long previousMillis_w;
unsigned long currentMillis_i;
unsigned long previousMillis_i;


float getMotionAvarage() {
  
  int AVG_READINGS_NUM = 20;  // Number of readings to take an avarage of, to determine the avarage motion displacement.
  float readingsSum = 0.0;
  for (int i = 0; i < AVG_READINGS_NUM; i++){

    /*  Get sensor readings. */
    // Read gyroscope data
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);

    // Calculate the magnitude of the gyroscope vector
    float gyroscopeMagnitude = sqrt(gx * gx + gy * gy + gz * gz);

    // Check if the gyroscope magnitude is a valid number
    if (isnan(gyroscopeMagnitude)) {
      return; 
    }

    // Get the absolute displacement value.
    absoluteDisplasement =  abs(previousReading - gyroscopeMagnitude);
    float absDisp2 = absoluteDisplasement * absoluteDisplasement;

    // Calculate the 

  }
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize the MPU6050 sensor
  Wire.begin();
  mpu.initialize();

  // Set gyroscope full-scale range (FSR)
  // Options: MPU6050_GYRO_FS_250, MPU6050_GYRO_FS_500, MPU6050_GYRO_FS_1000, MPU6050_GYRO_FS_2000 (Higher the value the more sensitive the Gyro)
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  Serial.print("Full range: ");
  Serial.println(mpu.getFullScaleGyroRange());
  Serial.println();
  delay(2000);
}

void loop() {
  // Read gyroscope data
  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  // Calculate the magnitude of the gyroscope vector
  float gyroscopeMagnitude = sqrt(gx * gx + gy * gy + gz * gz);

  // Check if the gyroscope magnitude is a valid number
  if (isnan(gyroscopeMagnitude)) {
    // Skip the rest of the loop and wait for the next reading
    return;
  }

  /* Calculate absolute displacement */
  absoluteDisplasement = abs(previousReading - gyroscopeMagnitude);
  float absDisp2 = absoluteDisplasement * absoluteDisplasement;

  /* Logic to check if state has changed */
  if (absDisp2 > WORKING_THRESHOLD_DISP) {
    // In Working state
    currentMillis_w = millis();
    if (abs(currentMillis_w - previousMillis_w) > WORKING_THRESHOLD_PERIOD) {
      Serial.print(abs(currentMillis_w - previousMillis_w));
      Serial.println(" WORKING STATE! ");
      // Set state
      // state = HIGH;
      previousMillis_w = currentMillis_w;
    }
    // Reset the idle state timer
    previousMillis_i = millis();
  } else if (absDisp2 > IDLE_THRESHOLD_DISP && absDisp2 < WORKING_THRESHOLD_DISP) {
    // In Idle state
    currentMillis_i = millis();
    if (abs(currentMillis_i - previousMillis_i) > IDLE_THRESHOLD_PERIOD) {
      Serial.print(abs(currentMillis_i - previousMillis_i));
      Serial.println(" IDLE STATE! ");
      // Set state
      // state = LOW;
      previousMillis_i = currentMillis_i;
      previousMillis_w = millis();
    }
  }
  // Check if OFF (To be implemented)

  // Set current reading to the previous value for the next iteration.
  previousReading = gyroscopeMagnitude;
  delay(50); // Refine to suit sampling frequency
}

