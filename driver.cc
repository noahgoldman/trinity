#define WF 0
#define L 1
#define R 2
#define I 3
#define U 4
#define S 5

const int left = -1, right = 1, uturn = 0;
int command;
float close;

// *************************************
// **       Wall Following            **
// *************************************
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

void parseSerial() {
  command = Serial.parseInt();
}

void sendSensors() {
  int lnew[5];
  lnew[0] = (robot.front() > close);
  // trivial
  // gg
}

void loop() {
  parseSerial();
  switch (command) {
    case WF:
      wallFollow();
      break;
    case L:
      robot.turn(left); 
      break;
    case R:
      robot.turn(right);
      break;
    case I:
      robot.drive();
      break;
    case U:
      robot.turn(uturn);
      break;
    case S:
      robot.stop();
      break;
  }
}
