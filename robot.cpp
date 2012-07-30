#include "robot.h"
#include <math.h>
#include <Wire.h>
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define MAG_ADDR 0x1E

const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const float center = 67;
int gyrozero = 216;
const float gyrorate = 1.55;

const int flame1 = 7;
const int fanpin = 25;
const int gyropin = A9, gyrozeropin = 13;
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
  float theta = fabs(this->calcAngle(distance1, distance2));
  float pdist = ((distance1+distance2)*cos(theta * DEG_TO_RAD))/2;
  Serial.print("front: ");
  Serial.print(distance1);
  Serial.print(" back: ");
  Serial.print(distance2);
  Serial.print(" angle: ");
  Serial.print(theta);
  Serial.print(" calculated: ");
  Serial.println(pdist);
  return pdist;
}

void Robot::turn(const int direction) {
  double angle = 0;
  unsigned long int time = millis();
  this->stop();
  if (direction == uturn) {
    this->caster(90);
    delay(500);
    this->motor(80,48);
    while(angle < 180) {
      double width = 1000 / (millis() - time);
      angle += (this->gyro() * gyrorate)/width; 
      time = millis();
      delay(5);
      Serial.println(angle);
    }
  }
  else {
    this->caster(90);
    delay(500);
    if (direction == right) {
      this->motor(80,48);
    }
    else if (direction == left) {
      this->motor(48,80);
    }
    while(angle < 90 && angle > -90) {
      double width = 1000 / (millis() - time);
      angle += (this->gyro() * gyrorate)/width; 
      time = millis();
      delay(5);
      Serial.println(angle);
    }
  }
  this->caster(0);
  this->stop();
  delay(1000);
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
  pinMode(gyrozeropin, OUTPUT);

  pinMode(fanpin, OUTPUT);

  this->configMagnetometer();    
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
void Robot::configGyro() {
  int avg = 0;
  for (int i = 0; i < 100; i++) {
    avg += analogRead(gyropin);
    delay(10);
  }
  gyrozero = avg / 100;
  Serial.println(gyrozero);
}

int Robot::gyro() {
  return (analogRead(gyropin) - gyrozero);
}
