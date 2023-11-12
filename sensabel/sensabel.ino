/**
 * Sensabel is the sport sabre for the digital age.
 * Piezo sensor on the analog pin
 * Acceleration and gyro sensor on the wire interface
 */
#include "Wire.h"

const int tapPin = A0;        // the piezo is connected to analog pin 0
const int tapThreshold = 100;  // threshold value to decide whethere there is a hit
const int MPU_ADDR = 0x68;    // I2C address of the accel/gyro sensor. If AD0 pin is set to HIGH, the I2C address will be 0x69.

int tapValue = 0;  // variable to store the value read from the sensor pin
int16_t accel[3];  // variables for accelerometer raw data
int16_t gyro[3];   // variables for gyro raw data
int16_t temperature; // variables for temperature data
float t, tp; // approximate timestamps from millis() for integration

// Vectors for accelerations, speeds and position or angle
float a[3], v[3], pos[3], alpha[3], omega[3], angle[3];
// Rotation matrix to get vectors into sabre frame of reference
const float asensor = 15.0; // Angle of sensor within the guard 
float rot[3][3]={{0,sin(asensor),cos(asensor)},{1,0,0},{0,cos(asensor),sin(asensor)}};

char tmp_str[7]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

void setup() {
  Serial.begin(9600);       // use the serial port for output
  
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
  tp=millis()/1000.0; // Initial timestamp
}

void loop() {
  tapValue = analogRead(tapPin);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers 

  t=millis()/1000.0; // Time after reading wire

  accel[0] = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accel[1] = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accel[2] = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro[0] = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro[1] = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro[2] = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)

  for (int i=0;i<3;i++) {
    a[i]=accel[i]/16000.0;
    alpha[i]=gyro[i]/16000.0;
  }
  
  Serial.print(" ax:");  Serial.print(a[0]);
  Serial.print(" ay:");  Serial.print(a[1]);
  Serial.print(" az:");  Serial.print(a[2]);
  Serial.print(" gx:");  Serial.print(alpha[0]);
  Serial.print(" gy:");  Serial.print(alpha[1]);
  Serial.print(" gz:");  Serial.print(alpha[2]);
  // Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);

  if (tapValue >= tapThreshold) {
    Serial.print(" tap "); Serial.print(tapValue/100.0);
  }
  Serial.println();
  tp=t; // Current time used in next loop
}
