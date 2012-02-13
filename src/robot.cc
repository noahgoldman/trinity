#include "pid.h"
#include "environment.h"
#include "robot.h"

double Robot::sp = 0;

Robot::Robot(const Environment &env, const int kp, const int ki, const int kd) : pid(&Input, &Output, &sp, kp, ki, kd, DIRECT) {
    environment = env;
}

void Robot::SetInput() {
    Input = environment.GetSensorRight() - environment.GetSensorLeft();
}

int main() 
{
    int i = 0;
    return i;
}
