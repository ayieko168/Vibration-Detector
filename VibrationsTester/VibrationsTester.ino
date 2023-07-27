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

  /*  --------------- ALGO1: Get the diff between the square of the magnitudes. ---------------  */
  // Get the absolute displacement value.
  absoluteDisplasement =  abs( (previousReading * previousReading) - (gyroscopeMagnitude * gyroscopeMagnitude) );

  // Print the gyroscope magnitude for debugging
  // Serial.print(previousReading);
  // Serial.print(",");
  // Serial.print(gyroscopeMagnitude);
  // Serial.print(",");
  // Serial.println(absoluteDisplasement);

  /*  --------------- ALGO2: Get the diff between the square of the magnitudes. ---------------  */
  // Get the absolute displacement value.
  absoluteDisplasement =  abs(previousReading - gyroscopeMagnitude);
  float absDisp2 = absoluteDisplasement * absoluteDisplasement;

  // Print the gyroscope magnitude for debugging
  // Serial.print("Gyroscope Magnitude: ");
  // Serial.print("previousReading :: ");
  Serial.print(previousReading);
  Serial.print(",");
  Serial.print(gyroscopeMagnitude);
  Serial.print(",");
  Serial.print(absoluteDisplasement);
  Serial.print(",");
  Serial.println(absDisp2);


  // Set current reading to previous value for next itteration.
  previousReading = gyroscopeMagnitude;
  delay(50); // Refine to suit sampling frequency
}
