// Copyright 2013, Pegasus Team

#include "./robot.h"

const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const int straight = front;
const float center = 67;
int gyrozero = 1331;
const float gyrorate = 4.25;

//HardWire Robot::Magneto(1,0);

// Assign the threshold to
Robot::Robot(const float close_threshold, const float distance_between,
    const int speed, const int turn_speed)
  : close(close_threshold),
  sensor_distance(distance_between),
  base_speed(speed),
  turn_speed(turn_speed) {}

const float Robot::getDistance(const int sensor) const {
  int voltage = analogRead(sensor);
  float distance;
  if (sensor == this->distance_front || sensor == this->distance_back) {
    distance = this->distanceRegression(voltage, 1);
  } 
  else if(sensor == this->right_front){
    distance = 51.668*pow(M_E, (-0.000541897 * voltage));
  }
  else {
    distance = this->distanceRegression(voltage, 0);
  }
  return distance;
}

const float Robot::distanceRegression(float voltage, int old) const {
  float distance;
  if (old) {
    distance = 64.2801 * pow(M_E, (-0.000670484 * voltage));
  } else {
    distance = 46.8015 * pow(M_E, (-0.000958948 * voltage));
  }
  return distance;
}

int Robot::open(const int direction) const {
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
    //this->led(right, HIGH);
    //this->led(left, LOW);
    return left;
  } else if (this->getDistance(right_front) < this->close &&
    this->getDistance(right_back) < this->close) {
    //this->led(left, HIGH);
    //this->led(right, LOW);
    return right;
  } else {
    //this->led(left, LOW);
    //this->led(right, LOW);
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
  } else if (direction == right) {
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
  } else if (direction == right) {
    distance1 = this->getDistance(right_front);
    distance2 = this->getDistance(right_back);
  } else if (direction == front) {
    return this->getDistance(distance_front);
  } else if (direction == back) {
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
  unsigned int time = millis();
  float current_angle = 0;
  if (this->wallFollowDir() == direction * -1) {
    current_angle = this->getAngle(direction * -1);
  }
  this->stop();
  if (direction == straight) {
    while (!this->wallFollowDir()) {
      this->caster(10);
      this->motor();
      delay(50);
    }
  } else if (direction == uturn) {
    this->led(straight, HIGH);
    this->caster(90);
    this->motor(64 + turn_speed, 64 - turn_speed);
    while (angle < 160) {
      double width = 1000 / (millis() - time);
      angle += (this->gyro() / gyrorate)/width;
      time = millis();
      delay(5);
    }
  } else {
    if (reverse) {
      this->caster(45 * direction * -1);
    } else {
      this->caster(45 * direction);
    }
    this->motorTurn(direction, reverse);

    int kturn_state = 0;
    while (angle < (85 + current_angle) && angle > (-85 + current_angle)) {
      if ((angle > 45 || angle < -45) && !kturn_state && reverse) {
        this->caster(45 * direction);
        delay(100);
        this->motorTurn(direction, 0);
        kturn_state = 1;
      }
      double width = 1000 / (millis() - time);
      angle += (this->gyro() / gyrorate)/width;
      SerialUSB.println(angle);
      time = millis();
      delay(5);
    }
  }
  this->caster(0);
  this->stop();
}

void Robot::turn_angle(const float target) {
  float angle = 0;
  unsigned int time = millis();
  this->stop();

  int direction;
  if (target > 0) {
    direction = right;
  } else {
    direction = left;
  }

  this->caster(45 * direction);
  this->motorTurn(direction, 0);

  while ((direction == left && angle > target) ||
      (direction == right && angle < target)) {
    double width = 1000 / (millis() - time);
    angle += (this->gyro() / gyrorate)/width;
    SerialUSB.println(angle);
    time = millis();
    delay(5);
  }
  this->stop();
  this->caster(0);
}

int Robot::checkTurnAngle(const float angle, const float target) {
  if (target < 0) {
    return (angle < target);
  } else {
   return (angle > target);
  }
}

void Robot::UV(const int direction) {
  this->tower(500*direction);
}

void Robot::caster(float angle) {
  this->caster_servo.write(center + angle);
}

void Robot::tower(float angle) {
  this->tower_servo.write(center - angle);
}

// This function simply sets the motors to the base_speed
void Robot::motor() {
  int dist = this->getDistance(this->distance_front);
  int speed;
  if (dist < 12) {
    speed = 64;
  }
  else if (dist < 30) {
    speed = 64 + ((2 * dist) - 34);
  } else {
    speed = base_speed;
  }
  this->motor(speed, speed);
}

void Robot::motor(int left, int right) {
  if (left == 64 && right == 64) {
    Serial2.write(byte(0));
  } else {
    Serial2.write(byte(left));
    Serial2.write(byte(127 + right));
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
  pinMode(obled, OUTPUT);
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
  Serial2.begin(9600);
  SerialUSB.begin();
  this->caster_servo.attach(caster_pin);
  this->tower_servo.attach(tower_pin);

  this->pinSetup();

  //this->configMagnetometer();
  digitalWrite(relay, LOW);
}

/*
void Robot::configMagnetometer() {
  //Set scale to +/- 1.3 Ga
  this->Magneto.beginTransmission(0x1E);
  this->Magneto.send(0x01); //ConfigurationRegisterB
  this->Magneto.send(0x01<<5); // Setting for scale is in top 3 bits of register
  this->Magneto.endTransmission();

  //Set continouous measurement
  this->Magneto.beginTransmission(0x1E);
  this->Magneto.send(0x02); //Address of ModeRegister
  this->Magneto.send(0x00); // Continous Measurement value
  this->Magneto.endTransmission();
}

float Robot::heading() {
  //raw values
  uint8 buffer[6];
  //concatenated values
  int x,y,z;
  //adjusted values
  float sx,sy,sz;

  //Receive information
  this->Magneto.beginTransmission(0x1E);
  this->Magneto.send(0x03); //Address of X msb register.
  this->Magneto.endTransmission();

  //Get raw values
  this->Magneto.requestFrom(0x1E, 6);
  if(6<=this->Magneto.available()){
    //Receive input from magnetometer
    for(uint8 i = 0; i < 6; i++){
      buffer[i]=this->Magneto.receive();
    }
  }

  //Concatenate data
  x= (buffer[0]<<8 ) | buffer[1]; //X
  z= (buffer[2]<<8 ) | buffer[3]; //Z
  y= (buffer[4]<<8 ) | buffer[5]; //Y

  //Maple uses 4 byte int, but magnetometer uses 2 complement 2 byte int
  x = (0x8000<x)? x-0x10000 : x;
  y = (0x8000<y)? y-0x10000 : y;
  z = (0x8000<z)? z-0x10000 : z;

  //Adjust for magnetic interference on pegasus
  x -= 10;
  y += 320;

  //Adjusts for scale
  sx = x*0.92;
  sy = y*0.92;
  sz = z*0.92;

  float heading = atan2(sy, sx);
  // Correct heading for declination value
  // obtained from magnetic-declination.com
  //heading-=0.2438;
  // Correct headings for negative values and wrap
  if (heading < 0) heading += 2*PI;
  if (heading > 2*PI) heading -= 2*PI;

  return heading*180/M_PI;
}
*/

float Robot::gyro() {
  return (analogRead(gyropin) - gyrozero);
}

void Robot::led(const int direction, const int state) {
  if (direction == right) {
    digitalWrite(red, state);
  } else if (direction == straight) {
    digitalWrite(blue, state);
  } else if (direction == left) {
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
