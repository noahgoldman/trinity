#ifndef SRC_ROBOT_H_
#define SRC_ROBOT_H

class Robot {
    public:
        explicit Robot();
        float get_distance(int sensor);
        float left();
        float right();
        float front();
        void fan();
        void turnright();
        void turnleft();
        void troll();
};

#endif
