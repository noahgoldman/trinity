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
        void uturn();
        float heading();
};
