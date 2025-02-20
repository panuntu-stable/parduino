#define DRIVER_STEP_TIME 2
#include <GyverStepper.h>
#include <TaskScheduler.h>
#define DAC_RESOLUTION (9)
#include <Adafruit_MCP4725.h>
Adafruit_MCP4725 dac;

//Defining voltage variable
long throttleVolt = 0;

//Pin definitions
const int pwmSteer = 0;
const int pwmThrottle = 7;
const int stepPin = 9;
const int dirPin = 8;
const int enablePin = 4;
const int brkPin = 5;

//Stepping motor setup
const int stepsPerRevilution = 400;
const int minSteps = 0;
const int maxSteps = stepsPerRevilution * 2;

//Stepping motor definitions
long currentPos = maxSteps/2;
long targetPos = maxSteps/2;
volatile long PulsesSteering = 0;
int PulseWidth = 0;

//Defining Pulses variable for Throttle
volatile long PulsesThrottle = 0;

//Time management
unsigned long previousMotorTime = millis();
long MotorInterval;
volatile long StartTimeSteering;
volatile long StartTimeThrottle;
volatile long CurrentTimeSteering;
volatile long CurrentTimeThrottle;

//GStepper function definition 
GStepper<STEPPER2WIRE> stepper(stepsPerRevilution, stepPin, dirPin); 

//Create the scheduler 
Scheduler runner;

//Function to call stepper.tick() - TaskScheduler will call this
void stepperTaskCallback() {
  stepper.tick();
}

//create a task that runs every 1 millisecond 
Task stepperTask(1, TASK_FOREVER, &stepperTaskCallback);

// Setup of main function
void setup() { 
  //Beginning serial communication
  Serial.begin(9600);
  
	//Pin modes
	pinMode(pwmSteer, INPUT_PULLUP);
  pinMode(pwmThrottle, INPUT_PULLUP);
	pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);
	pinMode(enablePin, OUTPUT);
  pinMode(brkPin, OUTPUT);
	digitalWrite(enablePin, LOW); // Enabling controller by default
	attachInterrupt(digitalPinToInterrupt(pwmSteer), PulseSteerTimer, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pwmThrottle), PulseThrottleTimer,CHANGE);
  stepper.setCurrent(currentPos);//Centering 


  //Beginning IIC comunication
  dac.begin(0x61);
  
	//Additional settings of GStepper library
	stepper.autoPower(true); //Power off when destination reached
	stepper.setRunMode(FOLLOW_POS);
	stepper.setMaxSpeed(4000);
	stepper.setAcceleration(4000);// 0 means instantly maximum speed
	

  //Setting and enabling task
  runner.addTask(stepperTask);
  stepperTask.enable();
}


//Main loop function
void loop() {
  Serial.println(currentPos);
  //Execute task if 1ms passed
  runner.execute();

  
  // Checking if pulses correct and mapping it to targetPos
	if (PulsesSteering >= 1100 && PulsesSteering <= 1900) {
	  targetPos = map(PulsesSteering, 1090, 1910, minSteps, maxSteps);
	}

  //Moving to the targetPos
	if (targetPos != currentPos && abs(targetPos - currentPos) > 30) {
		stepper.setTarget(targetPos);
		currentPos = targetPos;
	}
  // Checking if pulses are correct and mapping it to voltage
  if (PulsesThrottle >= 1100 && PulsesThrottle <=1900) {
    if (PulsesThrottle >= 1550) { 
      throttleVolt = map(PulsesThrottle, 1600, 1910, 0, 4095);
      digitalWrite(brkPin, LOW);

      }
    else {
      digitalWrite(brkPin, HIGH);
      throttleVolt = 0;
      }
      if (throttleVolt < 0 ) {
      throttleVolt = 0;
    }
    dac.setVoltage(throttleVolt, true);
  }
	
}
//Defining additional function for interrupts, reading steering pwm
void PulseSteerTimer(){
	CurrentTimeSteering = micros();
	if (CurrentTimeSteering > StartTimeSteering){
    if ( CurrentTimeSteering - StartTimeSteering >=1100 && CurrentTimeSteering - StartTimeSteering <=1900){
		PulsesSteering = CurrentTimeSteering - StartTimeSteering;
    }
		StartTimeSteering = CurrentTimeSteering;
	}
}
//Defining additional function for interrupts, reading throttle pwm
void PulseThrottleTimer(){
  CurrentTimeThrottle = micros();
  if (CurrentTimeThrottle > StartTimeThrottle){
    PulsesThrottle = CurrentTimeThrottle - StartTimeThrottle;
    StartTimeThrottle = CurrentTimeThrottle;
  }
}
