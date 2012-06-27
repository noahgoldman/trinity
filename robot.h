class Robot {
    public:
        explicit Robot(const float close_threshold);
        int left();
        int right();
        int front();
        void fan();
        void turn(int direction);
        float heading();
    private:
        get_distance(const int sensor);

        float close;
        int distance_left1;
        int distance_left2;
        int distance_right1;
        int distance_right2;
        int distance_front;
        int distance_back;
};
