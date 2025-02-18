#include <Arduino.h>
#line 1 "/home/pan/Arduino/interruptions/interruptions.ino"
#define DRIVER_STEP_TIME 2
#include <GyverStepper.h>

//Pin definitions
const int pwmSteer = 0;
const int stepPin = 9;
const int dirPin = 8;
const int enablePin = 4;
//Stepping motor setup
const int stepsPerRevilution = 400;
const int minSteps = 0;
const int maxSteps = stepsPerRevilution * 2;
//Stepping motor definitions
long currentPos = maxSteps/2;
long targetPos = 0;
volatile long Pulses = 0;
int PulseWidth = 0;
//Time management
unsigned long previousMotorTime = millis();
long MotorInterval;
volatile long StartTime;
volatile long CurrentTime;
//GStepper function definition 
GStepper<STEPPER2WIRE> stepper(stepsPerRevilution, stepPin, dirPin); 
// Setup of main function
#line 26 "/home/pan/Arduino/interruptions/interruptions.ino"
void setup();
#line 42 "/home/pan/Arduino/interruptions/interruptions.ino"
void loop();
#line 56 "/home/pan/Arduino/interruptions/interruptions.ino"
void PulseTimer();
#line 26 "/home/pan/Arduino/interruptions/interruptions.ino"
void setup() {
	//Pin modes
	pinMode(pwmSteer, INPUT_PULLUP);
	pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);
	pinMode(enablePin, OUTPUT);
	digitalWrite(enablePin, LOW); // Enabling controller by default
	attachInterrupt(digitalPinToInterrupt(pwmSteer), PulseTimer, CHANGE);
	//Additional settings of GStepper library
	stepper.autoPower(true); //Power off when destination reached
	stepper.setRunMode(FOLLOW_POS);
	stepper.setMaxSpeed(1000000);
	stepper.setAcceleration(0);// 0 means instantly maximum speed
	stepper.setCurrent(currentPos);//Centering 
}
//Main loop function
void loop() {
	if (Pulses >= 1100 && Pulses <= 1900) {
		targetPos = map(Pulses, 1050, 1950, minSteps, maxSteps);
	}
	
	if (targetPos != currentPos) {
		stepper.setTarget(targetPos);
		currentPos = targetPos;
	}
	Serial.println(targetPos);
	stepper.tick();

}
//Defining additional function for interrupts
void PulseTimer(){
	CurrentTime = micros();
	if (CurrentTime > StartTime){
		Pulses = CurrentTime - StartTime;
		StartTime = CurrentTime;
	}
}

