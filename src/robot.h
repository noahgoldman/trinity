#ifndef SRC_ROBOT_H_
#define SRC_ROBOT_H

class Robot {
    public:
        explicit Robot(const Environment &environment);

    private:
        Environment environment;
        int GetError() const;
};

#endif
