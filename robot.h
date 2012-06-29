#include "robot.cc"
#include <Servo.h>

class Robot {
    public:
        explicit Robot(const float close_threshold);
        int left_open();
        int right_open();
        int front_open();
        void fan();
        void turn(int direction);
        float heading();
        float distance(const int direction);
        void UV(const int direction);
        void caster(float angle);
        void tower(float angle);
    private:
        get_distance(const int sensor);

        Servo caster;
        Servo tower;

        float close;
        int distance_left1;
        int distance_left2;
        int distance_right1;
        int distance_right2;
        int distance_front;
        int distance_back;
};
