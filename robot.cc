const int left = -1, right = 1, uturn = 0, forward = 2, back = 3;
const float center = 90;

// Assign the threshold to 
void Robot::Robot(const float close_threshold, distance_between)
  : close(close_threshold),
  sensor_distance(distance_between) {}

void Robot::getDistance(const int sensor) {
  // TODO implement like it was in the old code
  pass;
}

int Robot::left_open() { 
  return (Robot.get_distance(distance_left1) > Robot.close &&
    Robot.get_distance(distance_left2) > Robot.close);
}

int Robot::right_open() {
  return (Robot.get_distance(distance_right1) > Robot.close &&
    Robot.get_distance(distance_right2) > Robot.close);
}

int Robot::front_open() {
  return (Robot.get_distance(distance_front) > Robot.close);
}

float Robot::distance(const int direction) {
  float distance1, distance2;
  if (direction == left) {
    distance1 = Robot.getDistance(distance_left1);
    distance2 = Robot.getDistance(distance_left2);
  }
  else if (direction == right) {
    distance1 = Robot.getDistance(distance_right1);
    distance2 = Robot.getDistance(distance_right2);
  }
  else if (direction = front) {
    return Robot.getDistance(distance_front);
  }
  else if (direction = back) {
    return Robot.getDistance(distance_back);
  }
  theta = atan( (distance1 - distance2) /sensor_distance);
  pdist = ((distance1+distance2)*cos(theta))/2;
  return pdist;
}

void Robot::turn(const int direction) {
  float initial_angle = Robot.heading();
  if (direction = uturn) {
    Robot.caster(500*direction)
    Robot.motor(80, 48); // TODO add real numbers
    while (Robot.heading() < initial_angle + 180*direction) {}
    Robot.caster(center);
    Robot.motor(80, 80);
  else {
    Robot.caster(500*direction);
    Robot.motor(80, 48); // TODO add real numbers
    while (Robot.heading() < initial_angle + 90*direction) {}
    Robot.caster(center);
    Robot.motor(80, 80);
}

void Robot::UV(const int direction) {
  Robot.tower(500*direction)
}

void Robot::caster(float angle) {
  caster.write(center + angle);
}

void Robot::tower(float angle) {
  tower.write(center + angle);
}
