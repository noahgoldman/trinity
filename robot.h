#include <Servo.h>

class Robot {
    public:
        explicit Robot(const float close_threshold, 
            const float distance_between);
        int left_open();
        int right_open();
        int front_open();
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
    private:
        float getDistance(const int sensor);
        float calcAngle(float distance1, float distance2);
        void configMagnetometer();

        Servo caster_servo;
        Servo tower_servo;

        float close;
        float sensor_distance;
        int base_speed;
        int distance_left1;
        int distance_left2;
        int distance_right1;
        int distance_right2;
        int distance_front;
        int distance_back;
};
