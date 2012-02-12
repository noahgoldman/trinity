#include "src/robot.h"

Robot::Robot(const Environment &env) {
    environment = env;
}

Robot::GetError() {
    int error = environment.GetSensorRight() - GetSensorLeft();
    return error;
}


