#include <EEPROM.h>

/*

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+                                                       + 
+  Program: Sequential Turn Signal Master               +
+  Version: 4.9.3zxy                                    +
+  Purpose: Using a 3 taillight per side configuration  +
+           this program will make them flash           +
+           sequentially for turn signal and brake.     +
+           4.8.2 and later supports tap signaling.     +
+           4.8.4 and Later supports tap mode change.   +
+           4.9.3 and later utilizes brake interrupt.   +
+  Author: Jason Saler                                  +
+  First Created: 05/06/2013                            +
+  Date Last Edited: 11/17/2018                         +
+                                                       +
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*/

// Define the input pins: Pin 1 will be right input, Pin 2 will be left input and Pin 3 will be brake input utilizing the interupt functionality
// 12 is Mode input which switches the program from sequential pattern to standard pattern
#define RIGHT_SIGNAL_INPUT 1 
#define LEFT_SIGNAL_INPUT 2 
#define BRAKE_INPUT 3

// Depending on the relay board this may need to be reversed
#define ON LOW
#define OFF HIGH

// Several standard time lengths in milliseconds
#define FAST_BLINK 100
#define QUICK_BLINK 200
#define MEDIUM_BLINK 250
#define LONG_BLINK 350
#define SLOW_BLINK 450
#define PAUSE_SECOND 1000

// The number of times a turning indicator sequence will run through on a tap 
#define MAX_REPEAT_ON_TAP 4

// Now declare the 8 light output pins, 4 per side. Variable names are quite descriptive
// Can use basically any pin, but note: Pin 13 has an LED connected on most Arduino boards.
const int leftMarkers = 4;
const int leftOuter = 5;
const int leftMiddle = 6; 
const int leftInner = 7;
const int rightInner = 8; 
const int rightMiddle = 9; 
const int rightOuter = 10; 
const int rightMarkers = 11;

// Constant value of the memory address used for storing and retrieving the mode
const int modeMemoryAddress = 0;

// Variables
bool isRightTap = false;
bool isLeftTap = false;
bool isEmergencyTap = false;
int flashCount = 0;
int mode = 0;
bool turnBrakeOn = false;
bool turnBrakeOff = false;

// I. the setup routine runs once when you press reset: This loop will reset all the variables to their starting type and state.
void setup() 
{                
  // Initialize the LEDs as outputs.
  pinMode(rightInner, OUTPUT); 
  pinMode(rightMiddle, OUTPUT);
  pinMode(rightOuter, OUTPUT);
  pinMode(leftInner, OUTPUT);
  pinMode(leftMiddle, OUTPUT);
  pinMode(leftOuter, OUTPUT);
  pinMode(rightMarkers, OUTPUT);
  pinMode(leftMarkers, OUTPUT);
  
  // Initialize the input pins as, well, inputs of course... 
  pinMode(RIGHT_SIGNAL_INPUT, INPUT);
  pinMode(LEFT_SIGNAL_INPUT, INPUT);
  pinMode(BRAKE_INPUT, INPUT_PULLUP);

  // Setup brake interupt
  attachInterrupt(digitalPinToInterrupt(BRAKE_INPUT), brakeInterrupt, CHANGE);
  
  // Set the initial inputs as HIGH. Could also be done as low, but then all the states should be reversed throughout the program: 1 to 0 and visa versa.
  digitalWrite(RIGHT_SIGNAL_INPUT, HIGH);
  digitalWrite(LEFT_SIGNAL_INPUT, HIGH);
  digitalWrite(BRAKE_INPUT, HIGH);

  mode = EEPROM.read(modeMemoryAddress);
  off();
}

// II. The loop routine runs over and over again forever(or at least as long as the board has power and is working...) 
//    This handles all of the actual light blinking and what not.
void loop() 
{
  // 1. First check if the brakes need to be turned on (when BRAKE_INPUT is 0).
  if((digitalRead(RIGHT_SIGNAL_INPUT) == 1) and (digitalRead(LEFT_SIGNAL_INPUT) == 1) and (digitalRead(BRAKE_INPUT) == 0) and (!isRightTap) and (!isLeftTap) and (!isEmergencyTap))
  {
    if(turnBrakeOn)
    {
      (mode == 0) ? (brakeSequenceFlashOn(FAST_BLINK)) : (brake());
      turnBrakeOn = false;
    }
    else
    {
      (mode == 0) ? (brakeSequenceHoldOn()) : (brake());
    }
  }
  
  // 2. Next the program will check if the brakes are now off and need to be turned off; if all inputs are 1, then nothing is on.  
  //    So nothing is lit up, set all the lights to LOW which is off.
  else if((digitalRead(RIGHT_SIGNAL_INPUT) == 1) and (digitalRead(LEFT_SIGNAL_INPUT) == 1) and (digitalRead(BRAKE_INPUT) == 1) and (!isRightTap) and (!isLeftTap) and (!isEmergencyTap) )
  { 
    // 2a. Check if brakes need to be turned off
    if(turnBrakeOff)
    {
      (mode == 0) ? (brakeSequenceFlashOff(FAST_BLINK)) : (off());
      turnBrakeOff = false;
    }
    resetFlashCount();
  } 
  
  // 3. Now checking for the emergency flashers being on and brake off, when this happens a fun litle flashing pattern is performed to warn other drivers of emergencies. If tapped the mode will change 
  else if( ( ( digitalRead(RIGHT_SIGNAL_INPUT) == 0 and digitalRead(LEFT_SIGNAL_INPUT) == 0 and !isLeftTap and !isRightTap) || (isEmergencyTap) ) and (digitalRead(BRAKE_INPUT) == 1) )
  {
    if(turnBrakeOff)
    {
      (mode == 0) ? (brakeSequenceFlashOff(FAST_BLINK)) : (off());
      turnBrakeOff = false;
    }
    else
    {
      (mode == 0) ? (emergencyFlasherSequence(FAST_BLINK, MEDIUM_BLINK, SLOW_BLINK)) : (emergencyFlasherBlink(SLOW_BLINK, MEDIUM_BLINK));
    }
  }

  // 4. Now checking for the emergency flashers being on and brake on, when this happens just light the brakes.
  else if( ( ( digitalRead(RIGHT_SIGNAL_INPUT) == 0 and digitalRead(LEFT_SIGNAL_INPUT) == 0 and !isLeftTap and !isRightTap) || (isEmergencyTap) ) and (digitalRead(BRAKE_INPUT) == 0) )
  {
    if(turnBrakeOn)
    {
      (mode == 0) ? (brakeSequenceFlashOn(FAST_BLINK)) : (off());
      turnBrakeOn = false;
    }
  }
  
  else
  {
    // 5. This is if the right turn signal is on, do a cycle through the lights.
    if(  (digitalRead(LEFT_SIGNAL_INPUT) == 1) and ( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) || (isRightTap) ) )
    {
      (mode == 0) ? (rightSequence(QUICK_BLINK, QUICK_BLINK)) : (rightBlink(SLOW_BLINK, MEDIUM_BLINK));
    }
    
    // 6. Here is if the left turn signal is on. Cycle through the lights.
    if( (digitalRead(RIGHT_SIGNAL_INPUT) == 1) and ( (digitalRead(LEFT_SIGNAL_INPUT) == 0) || (isLeftTap) ) )
    {
      (mode == 0) ? (leftSequence(QUICK_BLINK, QUICK_BLINK)) : (leftBlink(SLOW_BLINK, MEDIUM_BLINK));
    }
  }
}

// III. All constructive functions are found below.
//      Two options are given for each action, a regular blink or a sequential pattern.

void rightSequence(int durationMillis, int pauseDurationMillis)
{
  checkIsTap();

  if(digitalRead(BRAKE_INPUT) == 0)
  {
    selectRightOn(OFF, OFF, OFF, OFF);
    delay(pauseDurationMillis);
  }
  
  selectRightOn(ON, OFF, OFF, OFF);
  delay(durationMillis);

  selectRightOn(ON, ON, OFF, ON);
  delay(durationMillis);
  
  selectRightOn(ON, ON, ON, ON);
  delay(durationMillis);

  if(digitalRead(BRAKE_INPUT) == 1)
  {
    selectRightOn(OFF, OFF, OFF, OFF);
    delay(pauseDurationMillis);
  }

  updateTapControl();
}

void rightBlink(int durationMillis, int offDurationMillis)
{
  checkIsTap(); 

  if(digitalRead(BRAKE_INPUT) == 0)
  {
    selectRightOn(OFF, OFF, OFF, OFF);
    delay(offDurationMillis);
  }
  
  selectRightOn(OFF, ON, ON, ON);
  delay(durationMillis);
  
  if(digitalRead(BRAKE_INPUT) == 1)
  {
    selectRightOn(OFF, OFF, OFF, OFF);
    delay(offDurationMillis);
  }

  updateTapControl();
}

void leftSequence(int durationMillis, int pauseDurationMillis)
{
  checkIsTap(); 

  if(digitalRead(BRAKE_INPUT) == 0)
  {
    selectLeftOn(OFF, OFF, OFF, OFF);
    delay(pauseDurationMillis);
  }
  
  selectLeftOn(ON, OFF, OFF, OFF);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, OFF, ON);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, ON, ON);
  delay(durationMillis);

  if(digitalRead(BRAKE_INPUT) == 1)
  {
    selectLeftOn(OFF, OFF, OFF, OFF);
    delay(pauseDurationMillis);
  }

  updateTapControl();
}

void leftBlink(int onDurationMillis, int offDurationMillis)
{
  checkIsTap(); 

  if(digitalRead(BRAKE_INPUT) == 0)
  {
    selectLeftOn(OFF, OFF, OFF, OFF);
    delay(offDurationMillis);
  }
  
  selectLeftOn(OFF, ON, ON, ON);
  delay(onDurationMillis);
  
  if(digitalRead(BRAKE_INPUT) == 1)
  {
    selectLeftOn(OFF, OFF, OFF, OFF);
    delay(offDurationMillis);
  }

  updateTapControl();
}

void rightSequenceBrake()
{
  selectLeftOn(ON, ON, ON, OFF);
}

void rightBlinkBrake()
{
  selectLeftOn(OFF, ON, ON, OFF);
}

void leftSequenceBrake()
{
  selectRightOn(ON, ON, ON, OFF);
}

void leftBlinkBrake()
{
  selectRightOn(OFF, ON, ON, OFF);
}

void brakeSequenceFlashOn(int durationMillis)
{
  selectLeftOn(ON, OFF, OFF, OFF);
  selectRightOn(ON, OFF, OFF, OFF);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, OFF, OFF);
  selectRightOn(ON, ON, OFF, OFF);
  delay(durationMillis);

  selectLeftOn(ON, ON, ON, OFF);
  selectRightOn(ON, ON, ON, OFF);
}

void brakeSequenceFlashOff(int durationMillis)
{  
  selectLeftOn(ON, ON, OFF, OFF);
  selectRightOn(ON, ON, OFF, OFF);
  delay(durationMillis);

  selectLeftOn(ON, OFF, OFF, OFF);
  selectRightOn(ON, OFF, OFF, OFF);
  delay(durationMillis);

  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(OFF, OFF, OFF, OFF);
}

void brakeSequenceHoldOn()
{
  selectLeftOn(ON, ON, ON, OFF);
  selectRightOn(ON, ON, ON, OFF);
}

void brake()
{
  selectLeftOn(OFF, ON, ON, OFF);
  selectRightOn(OFF, ON, ON, OFF);
}

void emergencyFlasherSequence(int flashDurationMillis, int offDurationMillis, int onDurationMillis)
{
  checkIsTap();
  if(!isEmergencyTap)
  {
    brakeSequenceFlashOn(flashDurationMillis);  
    selectLeftOn(ON, ON, ON, ON);
    selectRightOn(ON, ON, ON, ON);
    delay(onDurationMillis);    
    
    brakeSequenceFlashOff(flashDurationMillis); 
  }
  else
  {
    delay(onDurationMillis);
  }
  off();
  delay(offDurationMillis);
  
  updateTapControl();
}

void emergencyFlasherBlink(int onDurationMillis, int offDurationMillis)
{
  checkIsTap();

  if(!isEmergencyTap)
  {
    selectLeftOn(OFF, ON, ON, ON);
    selectRightOn(OFF, ON, ON, ON);
  }
  delay(onDurationMillis);
  
  off();
  delay(offDurationMillis); 

  updateTapControl();
}

void off()
{
  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(OFF, OFF, OFF, OFF);
}

void leftOff()
{
  selectLeftOn(OFF, OFF, OFF, OFF);
}

void rightOff()
{
  selectRightOn(OFF, OFF, OFF, OFF);
}



//IIIa. Functions that handle the actual digital write actions.
//      Separate function for left side and right side.

void selectLeftOn(boolean  leftInnerPower, boolean leftMiddlePower, boolean leftOuterPower, boolean leftMarkersPower)
{
    digitalWrite(leftInner, leftInnerPower);   
    digitalWrite(leftMiddle,leftMiddlePower);
    digitalWrite(leftOuter, leftOuterPower);
    digitalWrite(leftMarkers, leftMarkersPower);
}

void selectRightOn(boolean rightInnerPower, boolean rightMiddlePower, boolean rightOuterPower, boolean rightMarkersPower)
{
    digitalWrite(rightInner, rightInnerPower);    
    digitalWrite(rightMiddle, rightMiddlePower);
    digitalWrite(rightOuter, rightOuterPower);
    digitalWrite(rightMarkers, rightMarkersPower);
}

// IIIb. Tap Logic Functions

void checkIsTap()
{ 
  if(flashCount > 0)
  {
    if( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) and (digitalRead(LEFT_SIGNAL_INPUT) == 0) )
    {
      isEmergencyTap = false;
      resetFlashCount();
    }    
    else if( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) )
    {
      isRightTap = false;
      resetFlashCount();
    }
    else if( (digitalRead(LEFT_SIGNAL_INPUT) == 0) )
    {
      isLeftTap = false;
      resetFlashCount();
    }
  }
  else
  {  
    if( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) and (digitalRead(LEFT_SIGNAL_INPUT) == 0) )
      isEmergencyTap = true;
    else if( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) )
      isRightTap = true;
    else if( (digitalRead(LEFT_SIGNAL_INPUT) == 0) )
      isLeftTap = true;
  }
}

void updateTapControl()
{
  flashCount++;
    
  if( (flashCount >= MAX_REPEAT_ON_TAP) and (isRightTap) )
  {
    isRightTap = false;
    resetFlashCount();
  }
  else if( (flashCount >= MAX_REPEAT_ON_TAP) and (isLeftTap) )
  {
    isLeftTap = false;
    resetFlashCount();
  }
  else if( (flashCount >= 2) and (isEmergencyTap) )
  {
    isEmergencyTap = false;
    resetFlashCount();

    (mode == 0) ? (mode = 1) : (mode = 0);
    
    EEPROM.write(modeMemoryAddress, mode);
  }
}

void resetFlashCount()
{
  flashCount = 0;
}

// IIIC. Brake Interupt Functions

void brakeInterrupt()
{
   // 1. Applying brakes
   if(digitalRead(BRAKE_INPUT) == 0)
   {
      // 1.1. Here is if the brake is applied alone or with emergency flashers on.
      if( ( digitalRead(RIGHT_SIGNAL_INPUT) == digitalRead(LEFT_SIGNAL_INPUT) && !isRightTap && !isLeftTap) || isEmergencyTap )
      { 
         turnBrakeOn = true;
      }
       
      // 1.2. Here is if the right turn signal and brake are on.
      else if( (digitalRead(LEFT_SIGNAL_INPUT) == 1) and ( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) || (isRightTap) ) )
      {         
        (mode == 0) ? (rightSequenceBrake()) : (rightBlinkBrake()); 
      }
      
      // 1.3. Now if the left turn signal and brake are both on
      else if( (digitalRead(RIGHT_SIGNAL_INPUT) == 1) and ( (digitalRead(LEFT_SIGNAL_INPUT) == 0) || (isLeftTap) ) )
      {
        (mode == 0) ? (leftSequenceBrake()) : (leftBlinkBrake());               
      }
    } 
    // 2. Releasing Brakes
    else
    {
      // 2.1. Here is if the brake is released with emergency flasher on or with no turn indicators on.
      if( ( digitalRead(RIGHT_SIGNAL_INPUT) == digitalRead(LEFT_SIGNAL_INPUT) && !isRightTap && !isLeftTap ) || isEmergencyTap )
      { 
         turnBrakeOff = true; 
      }
       
      // 2.2. Here is if the right turn signal is on and brake is released.
      else if( (digitalRead(LEFT_SIGNAL_INPUT) == 1) and ( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) || (isRightTap) ) )
      {         
        leftOff();
      }
      
      // 2.3. Now if the left turn signal is on and brake aris released.
      else if( (digitalRead(RIGHT_SIGNAL_INPUT) == 1) and ( (digitalRead(LEFT_SIGNAL_INPUT) == 0) || (isLeftTap) ) )
      {
        rightOff();              
      }
   }
}

