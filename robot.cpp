#include "robot.h"
#include <math.h>

const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const float center = 90;

// Assign the threshold to 
Robot::Robot(const float close_threshold, const float distance_between)
  : close(close_threshold),
  sensor_distance(distance_between) {}

float Robot::getDistance(const int sensor) {
  // TODO implement like it was in the old code
}

int Robot::left_open() { 
  return (this->getDistance(distance_left1) > this->close &&
    this->getDistance(distance_left2) > this->close);
}

int Robot::right_open() {
  return (this->getDistance(distance_right1) > this->close &&
    this->getDistance(distance_right2) > this->close);
}

int Robot::front_open() {
  return (this->getDistance(distance_front) > this->close);
}

float Robot::distance(const int direction) {
  float distance1, distance2;
  if (direction == left) {
    distance1 = this->getDistance(distance_left1);
    distance2 = this->getDistance(distance_left2);
  }
  else if (direction == right) {
    distance1 = this->getDistance(distance_right1);
    distance2 = this->getDistance(distance_right2);
  }
  else if (direction == front) {
    return this->getDistance(distance_front);
  }
  else if (direction == back) {
    return this->getDistance(distance_back);
  }
  float theta = atan( (distance1 - distance2) /sensor_distance);
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
  caster_servo.write(center + angle);
}

void Robot::tower(float angle) {
  tower_servo.write(center + angle);
}
