#ifndef ROBOT_H
#define ROBOT_H 

#include <Servo.h>

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class Robot {
    public:
        explicit Robot(const float close_threshold, 
            const float distance_between, const int speed);
        int open(const int direction);
        float getAngle(const int direction);
        float distance(const int direction);
        void fan();
        void turn(int direction);
        int heading();
        void UV(const int direction);
        void caster(float angle);
        void tower(float angle);
        void motor();
        void motor(int left_motor, int right_motor);
        void stop();
        void drive(int time);
        int flame();
        void setup();
        float getDistance(const int sensor);
        int gyro();
        int wallFollowDir();
    private:
        float calcAngle(float distance1, float distance2);
        float distanceRegression(float voltage);
        void configMagnetometer();
        int writeRegister(int deviceAddress, byte address, byte val);
        int readRegister(int deviceAddress, byte address);
        void configGyro(int scale);

        Servo caster_servo;
        Servo tower_servo;

        float close;
        float sensor_distance;
        int base_speed;
};

#endif
