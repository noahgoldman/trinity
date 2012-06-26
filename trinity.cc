#include <math.h>
#define PI 3.1415926535897932384626433832795028841971693993751058209749

// Sensor constants
const int df, dl1, dl2, dr1, dr2;
const int fl, fr;

// Operational constants
const int left = -1, right = 1;
const float center = 1500;
const float ideal = 20;
const float kPwall;
const float sensor_distance;

// Convert radians to degrees
inline float radians_to_degrees(float radians) { return ((180 * radians) / PI)}

inline float degrees_to_microseconds(float degrees) { 
  return ((500 * degrees) / 180)
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

// Turning and navigational logic works in the following manner (order is very important):
// 1. If the UVtrons on either side detect light, turn toward it
// 2. If the front distance is small, make a U-turn.  This should only occur when there is no light coming
//      from the rooms on either side
// 3. If all sides are open (the four corners) always make a left.
void check_turn() {
    if (robot.UV_left()) {
        robot.turnleft();
    }
    else if (robot.UV_right()) {
        robot.turnright();
    }
    else if (robot.front() < 20) {
        robot.uturn();
    }
    else if (robot.front() > 20 && robot.left() > 20 && robot.right() > 20) {
        robot.turnleft();
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

// The main event loop for the robot should function in the following manner.
//      -If the robot has not exited the initial room yet, continue to do so.
//      -When the robot exits the room, it should make an immediate right turn
//      -The navigate function should be called continuously until the ir sensor is activated
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
