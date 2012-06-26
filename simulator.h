class Simulator : public Robot {
 public:
   explicit Simulator(int robot_size) : size(robot_size) {}

 private:	 
   int x;
   int y;
   const int size;   
