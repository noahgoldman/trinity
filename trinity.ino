#include <math.h>
#define PI 3.1415926535897932384626433832795028841971693993751058209749

// These paths actually are true and should be used
int room1[] = {left, uturn, left, uturn, straight, left, left};
int room2[] = {left, left, uturn, straight, left, left};
int room3[] = {left, right, uturn, right, left, left};
int room4[] = {right, left, uturn, left};
int room5[] = {right, left, uturn, straight, left, left};
int room6[] = {left, left, straight, uturn, left};

int path[10];
int step = 0;

// These two constants are true if either uv or line is detected
volatile int uv, line;

// Sensor constants
const int df, dl1, dl2, dr1, dr2;
const int fl, fr;

// Operational constants
const int left = -1, right = 1, uturn = 0;
const float center = 1500;
const float ideal = 20;
const float kPwall;
const float sensor_distance;
const float close = 20;

// Convert radians to degrees
inline float radians_to_degrees(float radians) { return ((270 * radians) / PI)}

// TODO put this in robot.cc ><
float getAngle(int front, int back) {
  return atan((sensor_distance) / (back - front))
}

// Primary wall following function
void wallFollow(int dir) {
  float angle = getAngle(dir);
  float error = getWfError(dir);
  robot.caster(degrees_to_microseconds(angle) + error);
}

// Calculates the wall following error. Multiplying by the direction (1 or -1) 
//      should correctly adjust for wall following side
float getWfError(dir) {
  return ((robot.distance(dir) - ideal)*dir * kPWall);
}

// Turning and navigational logic works in the following manner (order is very 
//    important):
void check_turn() {
  // If all sides are open (four corners) then the next step in the path should
  //    be followed
  if (robot.front_open() && robot.left_open() && robot.right_open()) {
    // Turn according to the path
    robot.turn(path[step]); 
    step++;
  }
  // If the robot is about to crash, it probably shouldn't
  // Run the next step in the path if the front is closed
  else if (!robot.front_open()) {
    robot.turn(path[step]);
    step++;
  }
  // The next two cases handle when a side is open. You should only turn into
  // the side if the uv tron has activated
  else if (robot.left_open()) {
    delay(check_time);
    robot.UV(left);
    if (uv) {
      enter(left);
    }
  }
  else if (robot.right_open()) {
    delay(check_time);
    robot.UV(right);
    if (uv) {
      enter(right);
    }
  }
}

void enter(direction) {
  robot.turn(direction);
  while (!room) {
    robot.drive();
  }
  robot.stop();
}

// The primary navigation function that should be used when navigating the maze.
//      This should not be called while inside of a room
void navigate() {
  check_turn();
  wall_follow();
}

// This is the interrupt handler for the line sensor
ISR(ANALOG_COMP_vect) {
  if (!initial_exit) {initial_exit = 1}
  else {room = !room}
}

// This function will return the correct path to follow based on heading
//    and wall distances
//
// TODO the function returns a pointer to the path array, this should likely be
//    reworked
int *getPath() {
  float heading = robot.heading();
  if ((270 - path_margin) > heading && heading < (270 + path_margin)) {
    return room1;
  }
  else if ((90 - path_margin) < heading && heading < (90 + path_margin)) {
    return room2;
  }
  else if ((180 - path_margin) < heading && heading < (180 + path_margin)) {
    return room3[];
  }
  else if ((360 - path_margin) < heading || heading > (path_margin)) {
    return room4;
  }
}

// This function should be called directly at the beginning of execution
//    it drives forward until it hits a wall, then turns left
void start() {
  while (robot.front_open()) {
    drive();
  }
  robot.turn(left);
}

// This is looped continuously you leave the initial room
//
// The robot should simply wall follow forward until it hits a wall
//    then turn left
void escape() {
  if (!robot.front_open()) {
    robot.turn(left);
  }
  wallFollow();
}

void setup() {
  // Analog comparator stuff
  ACSR = B01011010;
}

// The main event loop for the robot should function in the following manner.
//      -If the robot has not exited the initial room yet, continue to do so.
//      -When the robot exits the room, it should make an immediate right turn
//      -The navigate function should be called continuously until the ir 
//         sensor is activated
//      -The extinguish function will be called until the flame is out
void loop() {
  check_ir();
  if (!initial_exit) {
    escape();
  }
  else if (room) {
    extinguish();
  }
  else {
    drive();
  }
}
