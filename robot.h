#include <Servo.h>

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
    private:
        float getDistance(const int sensor);
        float calcAngle(float distance1, float distance2);
        void configMagnetometer();

        Servo caster_servo;
        Servo tower_servo;

        float close;
        float sensor_distance;
        int base_speed;
};
