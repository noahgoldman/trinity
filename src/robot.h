#ifndef SRC_ROBOT_H_
#define SRC_ROBOT_H

class Robot {
    public:
        explicit Robot(const Environment&, int, int, int);

    private:
        PID pid;
        double Input, Output;
        static double sp;

        Environment environment;

        void SetInput();
};

#endif
