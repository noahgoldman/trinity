#include "robot.h"
#include <math.h>
#include <Wire/Wire.h>

#define MAG_ADDR 0x1E

const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const int straight = front;
const float center = 67;
int gyrozero = 216;
const float gyrorate = 1.55;

const int flame1 = 7;
const int fanpin = 25;
const int gyropin = 9, gyrozeropin = 13;
const int left_back = 0, left_front = 1, right_back = 2, right_front = 3,
      distance_front = 4, distance_back = 5;
const int caster_pin = 9, tower_pin = 11;
const int uvtron = 3, line = 2, start = 29;


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
      return ((this->getDistance(left_front) > this->close) &&
        (this->getDistance(left_back) > this->close));
      break;
    case right:
      return ((this->getDistance(right_front) > this->close) &&
        (this->getDistance(right_back) > this->close));
      break;
    case front:
      return (this->getDistance(distance_front) > (this->close - 15));
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
  else if (this->getDistance(right_front) < this->close &&
    this->getDistance(right_back) < this->close) {
    return right;
  }
  else {
    return 0;
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
  SerialUSB.println(angle);
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
  SerialUSB.print("front: ");
  SerialUSB.print(distance1);
  SerialUSB.print(" back: ");
  SerialUSB.print(distance2);
  SerialUSB.print(" angle: ");
  SerialUSB.print(theta);
  SerialUSB.print(" calculated: ");
  SerialUSB.println(pdist);
  return pdist;
}

void Robot::turn(const int direction) {
  double angle = 0;
  unsigned long int time = millis();
  float current_angle = 0;
  if (this->wallFollowDir() == direction * -1) {
    current_angle = this->getAngle(direction * -1);
  }
  this->stop();
  if (direction == straight) {
    delay(2000);
    while(!this->wallFollowDir()) {
      this->caster(0);
      this->motor();
    }
  }
  if (direction == uturn) {
    this->caster(90);
    this->motor(80,48);
    while(angle < 180) {
      double width = 1000 / (millis() - time);
      angle += (this->gyro() * gyrorate)/width; 
      time = millis();
      delay(5);
      SerialUSB.println(angle);
    }
  }
  else {
    this->caster(45 * direction);
    delay(500);
    if (direction == right) {
      this->motor(80,64);
    }
    else if (direction == left) {
      this->motor(64,80);
    }
    while(angle < (90 + current_angle) && angle > (-90 + current_angle)) {
      double width = 1000 / (millis() - time);
      angle += (this->gyro() * gyrorate)/width; 
      time = millis();
      delay(5);
      SerialUSB.println(angle);
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
    SerialUSB.write(byte(0));
  }
  else {
    SerialUSB.write(byte(left));
    SerialUSB.write(byte(127 + right));
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
  Wire().beginTransmission(MAG_ADDR);
  Wire().send(0x02);
  Wire().send(0x00);
  Wire().endTransmission();
}

void Robot::setup() {
  Wire().begin();
  Serial1.begin(9600);
  this->caster_servo.attach(caster_pin);
  this->tower_servo.attach(tower_pin);
  pinMode(start, INPUT);
  pinMode(gyrozeropin, OUTPUT);
  pinMode(line, INPUT);

  pinMode(fanpin, OUTPUT);

  this->configMagnetometer();    
}

int Robot::heading() {
  int x, y, z;

  // Select the register to start reading data from
  Wire().beginTransmission(MAG_ADDR);
  Wire().send(0x03);
  Wire().endTransmission();

  // Read data from each axis.
  // All registers must be read even though we only use x and y
  Wire().requestFrom(MAG_ADDR, 6);
  if (6 <= Wire().available()) {
    // Combine the complement values to get the actual readings
    x = Wire().receive() << 8;
    x |= Wire().receive();
    z = Wire().receive() << 8;
    z |= Wire().receive();
    y = Wire().receive() << 8;
    y |= Wire().receive();
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

int Robot::gyro() {
  return (analogRead(gyropin) - gyrozero);
}
