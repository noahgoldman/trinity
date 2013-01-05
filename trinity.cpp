// Copyright 2012, Pegasus Team

#include <stdio.h>
#include <wirish/wirish.h>
#include <Servo/Servo.h>
#include <Wire/Wire.h>
#include "./robot.h"

#define END 10

// These two constants are true if either uv or line is detected
volatile int uv = 0, line, room = 0, initial_exit = 0;

// Operational constants
const int left = -1, right = 1, uturn = 0, front = 2, back = 3;
const int uv_left = 4, uv_right = 5;
const int straight = front;

float ideal = 13;
const float kPWall = 2;
const float sensor_distance = 17;
const float close = 22;
const int check_time = 0;
const int path_margin = 20;
const int speed = 90;
const int turn_speed = 26;
const int turn_delay = 1500;

int on;

int path[6][7] = {
  // Room 0
  {left, uv_right, uv_right, left, uv_right, straight, uv_left, left},
  // Room 1
  {left, left, uturn, straight, left, left, END},
  // Room 2
  {left, right, uturn, right, left, left, END},
  // Room 3
  {right, left, uturn, left, END, END, END},
  // Room 4
  {right, left, uturn, straight, left, left, END},
  // Room 5
  {left, left, straight, uturn, left, END, END},
};

int start_room = 0;
int step = 0;
unsigned int path_time;

Robot robot(close, sensor_distance, speed, turn_speed);

// Calculates the wall following error. Multiplying by the direction (1 or -1)
//      should correctly adjust for wall following side
float getWfError(const int dir) {
  return ((robot.distance(dir) - ideal)*dir * kPWall);
}

// Primary wall following function
void wallFollow() {
  int dir = robot.wallFollowDir();
  if (dir == 0) {
    robot.driveStraight();
  } else {
    float angle = robot.getAngle(dir);
    float error = getWfError(dir);
    robot.caster(angle + error);
    robot.motor();
  }
}

void enter(const int dir) {
  robot.turn(dir);
  while (!room) {
    robot.motor();
  }
  robot.stop();
}

int getNextStep() {
  return path[start_room][step];
}

void resetPathTime() {
  path_time = millis() + turn_delay;
}

// Turning and navigational logic works in the following manner (order is very
//    important):
int checkTurn() {
  if (path_time > millis()) {
    return 0;
  } else if (start_room == 2 && path[start_room][step] != uturn
      && (robot.open(front) && robot.open(right)
      && robot.getDistance(16) > close)) {
    // This is the situation for dealing with taking a right on the four corners
    // coming out of the double room hallway
    return 1; 
  } else if (start_room != 2 && path[start_room][step] != uturn
      && (robot.open(front) && robot.open(right) && robot.open(left))) {
    // If all sides are open (four corners) then the next step in the path
    //    should be followed
    // Turn according to the path
    return 1;
  } else if (!robot.open(front) || robot.open(left) || robot.open(right)) {
    // If the robot is about to crash, it probably shouldn't
    // Run the next step in the path if the front is closed
    return 1;
  }
  return 0;
}

void tryTurn() {
  int step = getNextStep();
  switch (step) {
    case left:
      if (robot.open(left)) {
        robot.turn(left); 
        ++step;
      } break;
    case right:
      if (robot.open(right)) {
        robot.turn(right);
        ++step;
      } break;
    case uturn:
      robot.turn(uturn);
      ++step;
      break;
    case straight:
      if (robot.open(front)) {
        robot.turn(straight);
        ++step;
      } break;
    case uv_left:
      if (robot.open(left)) {
        robot.UV(left);
        delay(check_time);
        if (uv) {
          // enter(left);
        } else if (!robot.open(front)) {
          robot.turn(uturn);
        }
        step++;
      } break;
    case uv_right:
      if (robot.open(right)) {
        robot.UV(right);
        delay(check_time);
        if (uv) {
          // enter(right);
        } else if (!robot.open(front)) {
          robot.turn(uturn);
        }
        step++;
      } break;
    default: break;
  }
  resetPathTime();
}

// The primary navigation function that should be used when navigating the maze.
//      This should not be called while inside of a room
void navigate() {
  if (checkTurn()) {
    tryTurn();
  }
  wallFollow();
}

// This is the interrupt handler for the line sensor
void ir() {
  line = 1;
}

void exit() {
  robot.driveStraight();
  delay(250);
  line = 0;

  while (robot.open(front) && !line) {
    if (!robot.wallFollowDir()) {
      wallFollow();
    } else {
      robot.driveStraight();
    }
  }
  int reverse = 0;
  if (line) {
    reverse = 1;
  }
  robot.turn(path[start_room][step], reverse);
  step++;

  if (line) path_time = millis() + 1000;
}

void interpret_ir() {
  if (line && !initial_exit) {
    exit();
    initial_exit = 1;
  }
  /*
  else if (line && !room) {
    room = 1;
  }
  */
  line = 0;
}

// This function will return the correct path to follow based on heading
//    and wall distances
void getPath() {
  float heading = robot.heading();
  if ((270 - path_margin) > heading && heading < (270 + path_margin)) {
    start_room = 0;
  } else if ((90 - path_margin) < heading && heading < (90 + path_margin)) {
    start_room = 1;
  } else if ((180 - path_margin) < heading && heading < (180 + path_margin)) {
    start_room = 2;
  } else if ((360 - path_margin) < heading || heading > (path_margin)) {
    start_room = 3;
  }
}

// This is looped continuously you leave the initial room
//
// The robot should simply wall follow forward until it hits a wall
//    then turn left
void escape() {
  if (!robot.open(front) && robot.open(left)) {
    robot.turn(left);
  }
  if (!robot.open(front) && robot.open(right)) {
    robot.turn(right);
  }
  wallFollow();
}


// This is the function that should be called upon entering the room
// with the candle
//
// Do a sweep for the max flame value then fire the fans
void extinguish() {
  int tower_angle = 1;
  int max = 0;
  while (tower_angle <= 90) {
    robot.tower(tower_angle);
    int current_flame = robot.flame();
    if (current_flame > max) {
      max = tower_angle;
    }
    tower_angle++;
    delay(10);
  }

  robot.tower(max - 10);
  delay(1000);
  robot.fan();
}

/*
void set_leds() {
  if (robot.wallFollowDir() == right) {
    robot.led(right, HIGH);
  }
  else {
    robot.led(right, LOW);
  }
  if (robot.wallFollowDir() == left) {
    robot.led(left, HIGH);
  }
  else {
    robot.led(left, LOW);
  }
}
*/

void setup() {
  on = 1;
  robot.setup();
  attachInterrupt(36, ir, RISING);
  resetPathTime();
}

// The main event loop for the robot should function in the following manner.
//      -If the robot has not exited the initial room yet, continue to do so.
//      -When the robot exits the room, it should make an immediate right turn
//      -The navigate function should be called continuously until the ir
//         sensor is activated
//      -The extinguish function will be called until the flame is out
void loop() {
  /*
  SerialUSB.print("front: ");
  SerialUSB.print(robot.getDistance(15));
  SerialUSB.print("right front: ");
  SerialUSB.print(robot.getDistance(18));
  SerialUSB.print(" right back: ");
  SerialUSB.print(robot.getDistance(19));
  SerialUSB.print(" left front: ");
  SerialUSB.print(robot.getDistance(16));
  SerialUSB.print(" left back: ");
  SerialUSB.println(robot.getDistance(17));
  delay(1000);
  */
  /*
  int trials = 1000000;
  unsigned long int count_front = 0;
  unsigned long int count_back = 0;
  for (int i = 0; i < trials; i++) {
    count_front += analogRead(16);
  }

  int avg_front = count_front / trials;
  SerialUSB.print("front: ");
  SerialUSB.print(avg_front);
  */
  interpret_ir();
  if (!initial_exit) {
    escape();
  }
  else if (room) {
    extinguish();
  }
  else {
    navigate();
  }
}

// This should do some kind of Wiring init thing that stops stuff from being bad
__attribute__((constructor)) void premain() {
  init();
}

// We now need to define a main function with the maple
int main(void) {
  setup();

  while (true) {
    loop();
  }

  return 0;
}

