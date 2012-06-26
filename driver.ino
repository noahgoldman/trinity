//Code for the driver controller

#include <Servo.h> 
#include <Wire.h> //I2C stuff

#define MAG_ADDR 0x0E //7-bit address for the magnetometer

//Pin number macros (magnetometer wired to I2C)
const int casterpin=9, leftpin=11, rightpin=5;
const int flir=A0, frir=A1, blir=A2, brir=A3, fir=A4;

//Constants
const int fast, slow, stop;
const int between, ideal, castercenter, wfk;

//Variables
Servo caster;

/***************************************
 *Input
 ***************************************/

//Reads input from distance sensor
double getdist(int ir) {
  return 1/(0.000139331*analogRead(ir)+0.00487296)-4.58425;
}

//Starts up magnetometer
void magconfig() {
  Wire.beginTransmission(MAG_ADDR); //transmit to magnetometer
  Wire.write(0x11); //cntrl register2
  Wire.write(0x80); //send 0x80, enable auto resets
  Wire.endTransmission();
  Wire.beginTransmission(MAG_ADDR); 
  Wire.write(0x10); //cntrl register2
  Wire.write(1); //send 0x01, active mode
  Wire.endTransmission();
}

//Reads input from magnetometer
int magread() {
  int z1, zh; //define the MSB and LSB
  Wire.beginTransmission(MAG_ADDR); 
  Wire.write(0x05); //z MSB reg
  Wire.endTransmission();
  delayMicroseconds(2);
  Wire.requestFrom(MAG_ADDR, 1); //request 1 byte
  while(Wire.available()) {
    zh = Wire.read();
  }
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x06); //z LSB reg
  Wire.endTransmission();
  delayMicroseconds(2);
  Wire.requestFrom(MAG_ADDR, 1); //request 1 byte
  while(Wire.available()) {
    zl = Wire.read();
  }
  int zout = (zl|(zh <<8)); //concatenate the MSB and LSB
  return zout;
}

/***************************************
 *Movement
 ***************************************/
//Writes to motor controller
//0 stopped, -127 full reverse, 128 full forward
void setmotor(int left, int right) {
  analogWrite(leftpin, left+127);
  analogWrite(rightpin, right+127);
}

void turnright() {
  int start;
  start = magread();
  setmotor(slow,-slow);
    while(magread()<start) {
    }
  setmotor(stop,stop);
}

void turnleft() {
  int start;
  start = magread();
  setmotor(-slow,slow);
    while(magread()<start) {
    }
  setmotor(stop,stop);
}

void uturn() {
  int start;
  start = magread();
  setmotor(slow,-slow);
    while(magread()<start) {
    }
  setmotor(stop,stop);
}

void wallfollow(int fsensor, int bsensor, int motorval, int reverse) {
  setMotor(motorval,motorval);
  double theta;
  theta=atan((getdistance(fsensor)-getdistance(bsensor))/between);
  error=ideal-cos(theta)*getdistance(fsensor);
  if(reverse){
    caster.writeMicroseconds(castercenter-wfk*error-theta*1000/3.141592);
  }
  else{
    caster.writeMicroseconds(castercenter+wfk*error-theta*1000/3.141592);
  }
}

/***************************************
 *Porcelain
 ***************************************/
void getout(){
}


/***************************************
 *Main
 ***************************************/

void setup(){
  Serial.begin(9600);
  pinMode(leftpin, OUTPUT);
  pinMode(rightpin, OUTPUT);
  caster.attach(casterpin);
  caster.writeMicroseconds(castercenter);
  Wire.begin();
  magconfig();
}

void loop(){

}
