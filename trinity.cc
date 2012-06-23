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

inline float degrees_to_microseconds(float degrees) { return ((500 * degrees) / 180)}

float getAngle(int front, int back) {
    return atan((sensor_distance) / (back - front))
}

// Primary wall following function
void wall_follow(int dir) {
    float angle = getAngle(dir);
    float error = get_wf_error(dir);
    robot.servo(degrees_to_microseconds(angle) + error);
}

// 
float get_wf_error(dir) {
    return ((robot.distance(dir) - ideal)*dir * kPWall);
}

void drive() {
    if (robot.ir()) {
        room = 1;
    }
    check_turn();
    wall_follow();
