// Copyright 2012, Pegasus Team

#ifndef ROBOT_H_
#define ROBOT_H_

#include <Servo/Servo.h>
#include <libmaple/i2c.h>

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
        void pinSetup();
        const float getDistance(const int sensor) const;
        float gyro();
        int wallFollowDir();
        void led(const int direction, const int state);
        void led_off();
        void driveStraight();

    private:
        float calcAngle(float distance1, float distance2);
        const float distanceRegression(float voltage, int old) const;
        void configMagnetometer();
        int writeRegister(int deviceAddress, byte address, byte val);
        int readRegister(int deviceAddress, byte address);
        void configGyro();
        void motorTurn(const int direction, int reverse);
        void i2cInitMessage(i2c_msg *msg, uint8 *data, int read);

        Servo caster_servo;
        Servo tower_servo;

        float close;
        float sensor_distance;
        int base_speed;
        int turn_speed;
};

#endif  // ROBOT_H_
