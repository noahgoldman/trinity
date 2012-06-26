//Code for the master controller

#include <Servo.h>

//Pin number macros (uvtron output on digital pin 2)
const int panpin=9, fanpin=7, startpin=5;
const int linepin=A0, bir=A1, lflame=A2, rflame=A3;

//Constants
const int pancenter;
const int distmargin, Adist, Bdist, Cdist, Ddist;
const int magmargin, north, south, east, west;

//Variables
int startroom;
int linestate;
double distance;

//Paths


void setup(){
Serial.begin(9600);
pinMode(fanpin,OUTPUT);
pinMode(startpin,INPUT);
pan.attach(panpin);
pan.writeMicroseconds(pancenter);
//attachInterrupt(0, , RISING); //uvtron interrupt
}

void loop(){

}
