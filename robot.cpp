#include "robot.h"
#include <math.h>
#include <Wire.h>
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#define MAG_ADDR 0x0E

const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const float center = 90;

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
  // TODO implement like it was in the old code
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

float Robot::calcAngle(float distance1, float distance2) {
  float theta = atan((distance1 - distance2) / this->sensor_distance);
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
  return this->calcAngle(distance1, distance2);
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
    this->caster(500*direction);
    this->motor(80, 48); // TODO add real numbers
    while (this->heading() < initial_angle + 90*direction) {}
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
  Wire.beginTransmission(MAG_ADDR); //transmit to magnetometer
  Wire.write(0x11); //cntrl register2
  Wire.write(0x80); //send 0x80, enable auto resets
  Wire.endTransmission();
  Wire.beginTransmission(MAG_ADDR); 
  Wire.write(0x10); //cntrl register2
  Wire.write(1); //send 0x01, active mode
  Wire.endTransmission(); 
}

void Robot::setup() {
  Serial1.begin(9600);
  this->caster_servo.attach(caster_pin);
  this->tower_servo.attach(tower_pin);
  pinMode(start, INPUT);

  pinMode(fanpin, OUTPUT);

  this->configMagnetometer();    
}

int Robot::heading() {
  int zl, zh; //define the MSB and LSB
  Wire.beginTransmission(MAG_ADDR); 
  Wire.write(0x05); //z MSB reg
  Wire.endTransmission();
  delayMicroseconds(2);
  Wire.requestFrom(MAG_ADDR, 1); //request 1 byte
  while(Wire.available()) {
    zh = Wire.read();
  }
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x06); //z LSB reg
  Wire.endTransmission();
  delayMicroseconds(2);
  Wire.requestFrom(MAG_ADDR, 1); //request 1 byte
  while(Wire.available()) {
    zl = Wire.read();
  }
  int zout = (zl|(zh <<8)); //concatenate the MSB and LSB
  return zout;
}
