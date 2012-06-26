#include <math.h>
#define PI 3.1415926535897932384626433832795028841971693993751058209749

// Awkward paths that dont really exist
int room1[] = {right, left};
int room2[] = {left, right};
int room3[] = {uturn, left};
int room4[] = {left, uturn};

int path[10];
int step = 0;

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

inline float degrees_to_microseconds(float degrees) { 
  return ((500 * degrees) / 270)
}

float getAngle(int front, int back) {
  return atan((sensor_distance) / (back - front))
}

// Primary wall following function
void wallFollow(int dir) {
  float angle = getAngle(dir);
  float error = get_wf_error(dir);
  robot.servo(degrees_to_microseconds(angle) + error);
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
  if (robot.front() > close && robot.left() > close && robot.right()) {
    // Turn according to the path
    robot.turn(path[step]); 
    step++;
  }
  // If the robot is about to crash, it probably shouldn't
  // Run the next step in the path if the front is closed
  else if (robot.front() < close) {
    robot.turn(path[step]);
    step++;
  }
  // The next two cases handle when a side is open. You should only turn into
  // the side if the uv tron has activated
  else if (robot.left() > close) {
    delay(check_time);
    robot.UVleft();
    if (uv) {
      enter(left);
    }
  }
  else if (robot.right() > close) {
    delay(check_time);
    robot.UVright();
    if (uv) {
      enter(right);
    }
  }
}

// The primary navigation function that should be used when navigating the maze.
//      This should not be called while inside of a room
void navigate() {
  check_turn();
  wall_follow();
}

// This function checks if the ir sensor has been activated
//      1. If the intial_exit var is still false, make it true
//      2. If its already true, do an xor on the room var
void check_ir() {
  int ir = robot.ir();
  if (!initial_exit) {initial_exit = 1}
  else {room ^= ir}
}

int *getPath() {
  float heading = robot.heading();
  if ((270 - path_margin > heading && heading < (270 + path_margin) {
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
