#include "robot.h"
#include <math.h>
#include <Wire/Wire.h>
#include <libmaple/i2c.h>

#define MAG_ADDR 0x1E
#define EMATH 2.718281828459045235360287

const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const int straight = front;
const float center = 67;
int gyrozero = 1324.9;
const float gyrorate = 4.25;

// Pins
const int flamepin = 20;
const int relay = 35;
const int gyropin = 12;
const int left_back = 17, left_front = 16, right_back = 19, right_front = 18,
      distance_front = 15, distance_back = 10;
const int caster_pin = 27, tower_pin = 28;
const int uvtron = 37, line = 36; 
const int red = 24, blue = 25, green = 26;

// Assign the threshold to
Robot::Robot(const float close_threshold, const float distance_between,
    const int speed, const int turn_speed)
  : close(close_threshold),
  sensor_distance(distance_between),
  base_speed(speed),
  turn_speed(turn_speed) {}

float Robot::getDistance(const int sensor) {
  int voltage = analogRead(sensor);
  float distance;
  if (sensor == distance_front || sensor == distance_back) {
    distance = this->distanceRegression(voltage, 1);
  }
  else {
    distance = this->distanceRegression(voltage, 0);
  }
  return distance;
}

float Robot::distanceRegression(float voltage, int old) {
  float distance;
  if (old) {
    distance = 66.2801 * pow(EMATH, (-0.000636283 * voltage));
  }
  else {
    distance = 52.6639 * pow(EMATH, (-0.00106323 * voltage));
  }
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
    return (this->getDistance(distance_front) > this->close - 3);
    break;
  case back:
    return (this->getDistance(distance_back) > this->close);
    break;
  }
    return 0;
  }

int Robot::wallFollowDir() {
  if (this->getDistance(left_front) < this->close &&
    this->getDistance(left_back) < this->close) {
    this->led(right, HIGH);
    this->led(left, LOW);
    return left;
  }
  else if (this->getDistance(right_front) < this->close &&
    this->getDistance(right_back) < this->close) {
    this->led(left, HIGH);
    this->led(right, LOW);
    return right;
  }
  else {
    this->led(left, LOW);
    this->led(right, LOW);
    return 0;
  }
}

float Robot::calcAngle(float distance1, float distance2) {
  float theta = atan((distance1 - distance2) / this->sensor_distance);
  theta *= -180/PI;
  return theta;
}

float Robot::getAngle(const int direction) {
  float distance1 = 0, distance2 = 0;
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
  float distance1 = 0, distance2 = 0;
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

void Robot::motorTurn(const int direction, int reverse) {
  if (reverse) {
    if (direction == right) this->motor(64, 64 - turn_speed);
    if (direction == left) this->motor(64 - turn_speed, 64);
  } else {
    if (direction == right) this->motor(64 + turn_speed, 64);
    if (direction == left) this->motor(64, 64 + turn_speed);
  }
}


void Robot::turn(const int direction) {
  this->turn(direction, 0);
}

void Robot::turn(const int direction, int reverse) {
  double angle = 0;
  unsigned long int time = millis();
  float current_angle = 0;
  if (this->wallFollowDir() == direction * -1) {
    current_angle = this->getAngle(direction * -1);
  }
  this->stop();
  if (direction == straight) {
    while(!this->wallFollowDir()) {
      this->caster(10);
      this->motor();
      delay(50);
    }
  }
  else if (direction == uturn) {
    this->caster(90);
    this->motorTurn(right, 0);
    while(angle < 160) {
        double width = 1000 / (millis() - time);
        angle += (this->gyro() / gyrorate)/width; 
        time = millis();
        delay(5);
        SerialUSB.println(angle);
    }
  }
  else {
    if (reverse) {
      this->caster(45 * direction * -1);
    } else {
      this->caster(45 * direction);
    }
    this->motorTurn(direction, reverse); 
    
    int kturn_state = 0;
    while(angle < (90 + current_angle) && angle > (-90 + current_angle)) {
      if ((angle > 45 || angle < -45) && !kturn_state && reverse) {
        this->caster(45 * direction);
        delay(100);
        this->motorTurn(direction, 0);
        kturn_state = 1;
      }
      double width = 1000 / (millis() - time);
      angle += (this->gyro() / gyrorate)/width; 
      time = millis();
      delay(5);
    }
  }
  this->caster(0);
  this->stop();
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
  return analogRead(flamepin);
}

void Robot::stop() {
  this->motor(64, 64);
}

void Robot::fan() {
  digitalWrite(relay, HIGH);
  delay(1750);
  digitalWrite(relay, LOW);
}

void Robot::pinSetup() {
  // Digital Inputs
  pinMode(uvtron, INPUT);
  pinMode(line, INPUT);

  // Digital Outputs
  pinMode(relay, OUTPUT);
  pinMode(caster_pin, PWM);
  pinMode(tower_pin, PWM);
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(BOARD_LED_PIN, OUTPUT);

  // Analog Inputs
  pinMode(distance_front, INPUT_ANALOG);
  pinMode(left_front, INPUT_ANALOG);
  pinMode(left_back, INPUT_ANALOG);
  pinMode(right_front, INPUT_ANALOG);
  pinMode(right_back, INPUT_ANALOG);
  pinMode(distance_back, INPUT_ANALOG);
  pinMode(flamepin, INPUT_ANALOG);
  pinMode(gyropin, INPUT_ANALOG);
}

void Robot::setup() {
  Serial1.begin(9600);
  SerialUSB.begin();
  this->caster_servo.attach(caster_pin);
  this->tower_servo.attach(tower_pin);

  this->pinSetup();

  this->configMagnetometer();    
}

void Robot::i2cInitMessage(i2c_msg *msg, uint8 *data, int read) {
  msg->addr = MAG_ADDR;
  if (read) {
    msg->flags = I2C_MSG_READ;
  } else {
    msg->flags = 0;
  }
  msg->data = data;
  msg->length = sizeof(data);
  msg->xferred = 0; 
}
   


void Robot::configMagnetometer() {
  i2c_msg settings;
  i2c_msg gain;
  i2c_msg continuous_measurement;
  uint8 measurement_msg[3];
  uint8 settings_msg[3];
  uint8 gain_msg[3];

  settings_msg[0] = 0x3C;
  settings_msg[1] = 0x00;
  settings_msg[2] = 0x70;

  gain_msg[0] = 0x3C;
  gain_msg[1] = 0x01;
  gain_msg[2] = 0xA0;

  measurement_msg[0] = 0x3C;
  measurement_msg[1] = 0x02;
  measurement_msg[2] = 0x00;

  this->i2cInitMessage(&settings, settings_msg, 0);
  this->i2cInitMessage(&gain, gain_msg, 0);
  this->i2cInitMessage(&continuous_measurement, measurement_msg, 0);

  i2c_master_enable(I2C1, 0);

  i2c_master_xfer(I2C1, &settings, 1, 0);
  i2c_master_xfer(I2C1, &gain, 1, 0);
  i2c_master_xfer(I2C1, &continuous_measurement, 1, 0);

  delay(10);
  this->heading();
}

int Robot::heading() {
  int x = 0, y = 0, z = 0;

  i2c_msg read_device[3];

  uint8 read_msg[2];
  uint8 read_msg_data[6];
  uint8 restart_register[2];

  read_msg[0] = 0x3D;
  read_msg[1] = 0x06;

  restart_register[0] = 0x3C;
  restart_register[0] = 0x03;

  this->i2cInitMessage(&read_device[0], read_msg, 0);
  this->i2cInitMessage(&read_device[1], read_msg_data, I2C_MSG_READ);
  this->i2cInitMessage(&read_device[2], restart_register, 0);


  i2c_master_xfer(I2C1, read_device, 3, 2);
  // Read data from each axis.
  // All registers must be read even though we only use x and y
  x = read_msg_data[0] << 8;
  x |= read_msg_data[1];
  z = read_msg_data[2];
  z |= read_msg_data[3];
  y = read_msg_data[4];
  y |= read_msg_data[5];

  SerialUSB.print("x: ");
  SerialUSB.print(x);
  SerialUSB.print(" y: ");
  SerialUSB.print(y);
  SerialUSB.print(" z: ");
  SerialUSB.print(z);

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

float Robot::gyro() {
  return (analogRead(gyropin) - gyrozero);
}

void Robot::led(const int direction, const int state) {
  if (direction == right) {
    digitalWrite(red, state);
  }
  else if (direction == straight) {
    digitalWrite(blue, state);
  }
  else if (direction == left) {
    digitalWrite(green, state);
  } 
}

void Robot::led_off() {
  digitalWrite(red, LOW);
  digitalWrite(blue, LOW);
  digitalWrite(green, LOW);
}

void Robot::driveStraight() {
  this->caster(0);
  this->motor();
}
