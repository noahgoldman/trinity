#include "robot.h"
#include <math.h>
#include <Wire.h>
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define MAG_ADDR 0x1E

#define GYRO_ADDR 105
// Gyro values
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const float center = 67;

const int flame1 = 7;
const int fanpin = 25;
const int left_back = A0, left_front = A1, right_back = A2, right_front = A3,
      distance_front = A4, distance_back = A5;
const int caster_pin = 9, tower_pin = 11;
const int uvtron = 3, line = 5, start = 29;


// Assign the threshold to
Robot::Robot(const float close_threshold, const float distance_between,
    const int speed)
  : close(close_threshold),
  sensor_distance(distance_between),
  base_speed(speed) {}

float Robot::getDistance(const int sensor) {
  int voltage = analogRead(sensor);
  float distance = this->distanceRegression(voltage);
  return distance;
}

float Robot::distanceRegression(float voltage) {
  float distance = -17.9294 * log(0.00212468 * voltage);
  return distance;
}

int Robot::open(const int direction) {
  switch (direction) {
    case left:
      return (this->getDistance(left_front) > this->close &&
        this->getDistance(left_back) > this->close);
      break;
    case right:
      return (this->getDistance(right_front) > this->close &&
        this->getDistance(right_back) > this->close);
      break;
    case front:
      return (this->getDistance(distance_front) > this->close);
      break;
    case back:
      return (this->getDistance(distance_back) > this->close);
      break;
  }
}

int Robot::wallFollowDir() {
  if (this->getDistance(left_front) < this->close &&
    this->getDistance(left_back) < this->close) {
    return left;
  }
  if (this->getDistance(right_front) < this->close &&
    this->getDistance(right_back) < this->close) {
    return right;
  }
}

float Robot::calcAngle(float distance1, float distance2) {
  float theta = atan((distance1 - distance2) / this->sensor_distance);
  theta *= -180/PI;
  return theta;
}

float Robot::getAngle(const int direction) {
  float distance1, distance2;
  if (direction == left) {
    distance1 = this->getDistance(left_front);
    distance2 = this->getDistance(left_back);
  }
  else if (direction == right) {
    distance1 = this->getDistance(right_front);
    distance2 = this->getDistance(right_back);
  }
  float angle = this->calcAngle(distance1, distance2);
  if (direction == right) angle *= -1;
  Serial.println(angle);
  return angle;
} 

float Robot::distance(const int direction) {
  float distance1, distance2;
  if (direction == left) {
    distance1 = this->getDistance(left_front);
    distance2 = this->getDistance(left_back);
  }
  else if (direction == right) {
    distance1 = this->getDistance(right_front);
    distance2 = this->getDistance(right_back);
  }
  else if (direction == front) {
    return this->getDistance(distance_front);
  }
  else if (direction == back) {
    return this->getDistance(back);
  }
  float theta = this->calcAngle(distance1, distance2);
  float pdist = ((distance1+distance2)*cos(theta))/2;
  return pdist;
}

void Robot::turn(const int direction) {
  float initial_angle = this->heading();
  if (direction == uturn) {
    this->caster(500*direction);
    this->motor(80, 48); // TODO add real numbers
    while (this->heading() < initial_angle + 180*direction) {}
    this->caster(center);
    this->motor(80, 80);
  }
  else {
    this->caster(90*direction);
    this->motor(80, 48); // TODO add real numbers
    int heading = this->heading();
    while (heading > initial_angle - 90*direction) {
      heading = this->heading();
      Serial.print("[magneto]");
      Serial.println(heading);
    }
    this->caster(center);
    this->motor(80, 80);
  }
}

void Robot::UV(const int direction) {
  this->tower(500*direction);
}

void Robot::caster(float angle) {
  this->caster_servo.write(center + angle);
}

void Robot::tower(float angle) {
  this->tower_servo.write(center + angle);
}

// This function simply sets the motors to the base_speed
void Robot::motor() {
  this->motor(base_speed, base_speed);
}

void Robot::motor(int left, int right) {
  if (left == 64 && right == 64) {
    Serial1.write(byte(0));
  }
  else {
    Serial1.write(byte(left));
    Serial1.write(byte(127 + right));
  }
}

int Robot::flame() {
  return analogRead(flame1);
}

void Robot::stop() {
  this->motor(64, 64);
}

void Robot::fan() {
  digitalWrite(fanpin, HIGH);
  delay(1750);
  digitalWrite(fanpin, LOW);
}

void Robot::configMagnetometer() {
  // Set the mode to continuous measurement
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();
}

void Robot::setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial1.begin(9600);
  this->caster_servo.attach(caster_pin);
  this->tower_servo.attach(tower_pin);
  pinMode(start, INPUT);

  pinMode(fanpin, OUTPUT);

  this->configMagnetometer();    
  this->configGyro(2000);
}

int Robot::heading() {
  int x, y, z;

  // Select the register to start reading data from
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x03);
  Wire.endTransmission();

  // Read data from each axis.
  // All registers must be read even though we only use x and y
  Wire.requestFrom(MAG_ADDR, 6);
  if (6 <= Wire.available()) {
    // Combine the complement values to get the actual readings
    x = Wire.read() << 8;
    x |= Wire.read();
    z = Wire.read() << 8;
    z |= Wire.read();
    y = Wire.read() << 8;
    y |= Wire.read();
  }


  // Both x and y need to have their centers adjusted and x's range 
  // is changed as well
  x -= 580;
  x *= -483/423;
  y += 135.5;
  z += 167;
  z *= 1367/1174;

  float heading = atan2(x, y);

  // Correct headings for negative values
  if (heading < 0) heading += 2*PI;

  return heading * 180/PI; 
}

int Robot::writeRegister(int deviceAddress, byte address, byte val) {
  Wire.beginTransmission(deviceAddress); // start transmission to device 
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}

int Robot::readRegister(int deviceAddress, byte address) {
    int v;
    Wire.beginTransmission(deviceAddress);
    Wire.write(address); // register to read
    Wire.endTransmission();

    Wire.requestFrom(deviceAddress, 1); // read a byte

    while(!Wire.available()) {
        // waiting
    }

    v = Wire.read();
    return v;
}

// Scale can be 250, 500, or 2000
void Robot::configGyro(int scale) {
  //From  Jim Lindblom of Sparkfun's code

  // Enable x, y, z and turn off power down:
  writeRegister(GYRO_ADDR, CTRL_REG1, 0b00001111);

  // If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
  writeRegister(GYRO_ADDR, CTRL_REG2, 0b00000000);

  // Configure CTRL_REG3 to generate data ready interrupt on INT2
  // No interrupts used on INT1, if you'd like to configure INT1
  // or INT2 otherwise, consult the datasheet:
  writeRegister(GYRO_ADDR, CTRL_REG3, 0b00001000);

  // CTRL_REG4 controls the full-scale range, among other things:

  if(scale == 250){
    writeRegister(GYRO_ADDR, CTRL_REG4, 0b00000000);
  }else if(scale == 500){
    writeRegister(GYRO_ADDR, CTRL_REG4, 0b00010000);
  }else{
    writeRegister(GYRO_ADDR, CTRL_REG4, 0b00110000);
  }

  // CTRL_REG5 controls high-pass filtering of outputs, use it
  // if you'd like:
  writeRegister(GYRO_ADDR, CTRL_REG5, 0b00000000);
}

int Robot::gyro() {
  byte zMSB = readRegister(GYRO_ADDR, 0x2D);
  byte zLSB = readRegister(GYRO_ADDR, 0x2C);
  return ((zMSB << 8) | zLSB);
}

