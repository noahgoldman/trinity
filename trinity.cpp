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
const int flame_max = 500;

int on;

int path[6][7] = {
  // Room 0
  {left, uturn, left, uturn, straight, left, left},
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

void resetPathTime() {
  path_time = millis() + turn_delay;
}

// Turning and navigational logic works in the following manner (order is very
//    important):
void checkTurn() {
  if (path_time < millis() && start_room == 2 && path[start_room][step] != uturn
      && (robot.open(front) && robot.open(right)
      && robot.getDistance(16) > close)) {
    robot.turn(path[start_room][step]);
    step++;
    resetPathTime();
  } else {
    // Check both sides for uv
    if (robot.open(left)) {
      // The next two cases handle when a side is open. You should only turn into
      // the side if the uv tron has activated
      robot.UV(left);
      delay(check_time);
      if (uv) {
        // enter(left);
      }
    }
    if (robot.open(right)) {
      robot.UV(right);
      delay(check_time);
      if (uv) {
       // enter(right);
      }
    } 

    // Remaining turning logic
    if (path_time < millis() && start_room != 2
        && path[start_room][step] != uturn
        && (robot.open(front) && robot.open(right) && robot.open(left))) {
      // If all sides are open (four corners) then the next step in the path
      //    should be followed
      // Turn according to the path
      robot.turn(path[start_room][step]);
      step++;
      resetPathTime();
    } else if (path_time < millis() && !robot.open(front)) {
      // If the robot is about to crash, it probably shouldn't
      // Run the next step in the path if the front is closed
      robot.turn(path[start_room][step]);
      step++;
      resetPathTime();
    }
  }
}


// The primary navigation function that should be used when navigating the maze.
//      This should not be called while inside of a room
void navigate() {
  checkTurn();
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
  int tower_angle = 100;
  int max = 0;
  int fire_angle = 0;
  while (tower_angle >= -90) {
    robot.tower(tower_angle);

    int current_flame = 0;
    for (int i = 0; i < 10; i++) {
      current_flame += robot.flame();
    }

    if (current_flame > max) {
      max = current_flame;
      fire_angle = tower_angle;
    }
    --tower_angle;
    delay(10);
  }

  fire_angle += 8;
  robot.tower(fire_angle);
  SerialUSB.println(fire_angle);
  delay(1000);

  robot.turn_angle(fire_angle);

  robot.motor();
  while (robot.flame() < flame_max) {}
  robot.stop();

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

