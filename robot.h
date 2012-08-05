#ifndef ROBOT_H
#define ROBOT_H 

#include <Servo/Servo.h>

class Robot {
    public:
        explicit Robot(const float close_threshold, 
            const float distance_between, const int speed);
        int open(const int direction);
        float getAngle(const int direction);
        float distance(const int direction);
        void fan();
        void turn(const int direction);
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
        void configGyro();

        Servo caster_servo;
        Servo tower_servo;

        float close;
        float sensor_distance;
        int base_speed;
};

#endif
