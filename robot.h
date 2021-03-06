// Copyright 2013, Pegasus Team

#ifndef ROBOT_H_
#define ROBOT_H_

#include <Servo/Servo.h>
#include <wirish/wirish.h>
#include <math.h>

class Robot {
    public:
        explicit Robot(const float close_threshold,
            const float distance_between, const int speed,
            const int turn_speed);
        int open(const int direction) const;
        float getAngle(const int direction);
        float distance(const int direction);
        void fan();
        void turn(const int direction);
        void turn(const int direction, int reverse);
        void turn_angle(const float angle);
        int checkTurnAngle(const float angle, const float target);
        //float heading();
        void UV(const int direction);
        void caster(float angle);
        void tower(float angle);
        void motor();
        void motor(int left_motor, int right_motor);
        void stop();
        void drive(int time);
        int flame();
        void setup();
        void pinSetup();
        const float getDistance(const int sensor) const;
        float gyro();
        int wallFollowDir();
        void led(const int direction, const int state);
        void led_off();
        void driveStraight();

        // Pins
        const static int flamepin = 2;
        const static int relay = 6;
        const static int gyropin = 10;
        const static int left_back = 15, left_front = 20, right_back = 17, right_front = 18,
              distance_front = 19, distance_back = 16;
        const static int caster_pin = 14, tower_pin = 11;
        const static int uvtron = 12, line = 34;
        const static int start_button = 3;
        const static int red = 37, blue = 35, green = 36;
        const static int obled = 13;

    private:
        float calcAngle(float distance1, float distance2);
        const float distanceRegression(float voltage, int old) const;
        //void configMagnetometer();
        int writeRegister(int deviceAddress, byte address, byte val);
        int readRegister(int deviceAddress, byte address);
        void configGyro();
        void motorTurn(const int direction, int reverse);

        Servo caster_servo;
        Servo tower_servo;
        //static HardWire Magneto;

        float close;
        float sensor_distance;
        int base_speed;
        int turn_speed;
};

#endif  // ROBOT_H_
