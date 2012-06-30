#include <Servo.h>

class Robot {
    public:
        explicit Robot(const float close_threshold, 
            const float distance_between);
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
        void motor(int left_motor, int right_motor);
    private:
        float getDistance(const int sensor);

        Servo caster_servo;
        Servo tower_servo;

        float close;
        float sensor_distance;
        int distance_left1;
        int distance_left2;
        int distance_right1;
        int distance_right2;
        int distance_front;
        int distance_back;
};
