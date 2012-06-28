/*
Code to interface with the analog comparator.
The analog comparator output is pulled low
if voltage applied by positive input < negative input
in this code, the positive input is the bandgap reference voltage (around 1.1V)

None of this will make any sense unless you 
 1) Read pages 271 through 274 on the Atmega2560 datasheet
 2) Know that AIN0 is disconnected, and AIN1 is D5 on the arduino mega.
*/

void setup() {
pinMode(5,INPUT);
Serial.begin(9600);

//Write to the Analog Comparator Control and Status Register
ACSR = B01011010
/*
bit 7: 0 enables analog comparator. 
       1 switches it off.
bit 6: 1 sets positive input to bandgap reference voltage
       0 sets positive input to AIN0 (which is not connected on the arduino)
bit 5: analog comparator output
bit 4: set to 1 to clear interrupt flag
bit 3: 1 enable comparator interrupt
       0 disable comparator interrupt
bit 2: 0 no connection between comparator and input capture function
       1 enable input capture function
bit 1and0: 10 sets interrupt to falling output edge
           00 output toggle
           01 reserved
           10 falling output edge
           01 rising output edge
*/
}

void loop() {
}

//Interrupt service request for the analog comparator
ISR(ANALOG_COMP_vect) {
Serial.println("Interrupt Executed!");
}
