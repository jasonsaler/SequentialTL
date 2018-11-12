#include <EEPROM.h>

/*

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+                                                       + 
+  Program: Sequential Turn Signal Master               +
+  Version: 4.8.4zxy                                    +
+  Purpose: Using a 3 taillight per side configuration  +
+           this program will make them flash           +
+           sequentially for turn signal and brake.     +
+           4.8.2 and later supports tap signaling.     +
+           4.8.4 and Later supports tap mode change.   +
+                                                       +
+  Author: Jason Saler                                  +
+  First Created: 05/06/2013                            +
+  Date Last Edited: 09/18/2018                         +
+                                                       +
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*/

// Define the input pins: Pin 9 will be right input, Pin 10 will be left input and Pin 11 will be brake input 
// 12 is Mode input which switches the program from sequential pattern to standard pattern
#define RIGHT_SIGNAL_INPUT 9 
#define LEFT_SIGNAL_INPUT 10 
#define BRAKE_INPUT 11 

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
const int leftMarkers = 1;
const int leftOuter = 2;
const int leftMiddle = 3; 
const int leftInner = 4;
const int rightInner = 5; 
const int rightMiddle = 6; 
const int rightOuter = 7; 
const int rightMarkers = 8;

// Constant value of the memory address used for storing and retrieving the mode
const int modeMemoryAddress = 0;

// Variables
bool isRightTap = false;
bool isLeftTap = false;
bool isEmergencyTap = false;
int flashCount = 0;
int mode = 0;

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
  pinMode(BRAKE_INPUT, INPUT);
  
  // Set the initial inputs as HIGH. Could also be done as low, but then all the states should be reversed throughout the program: 1 to 0 and visa versa.
  digitalWrite(RIGHT_SIGNAL_INPUT, HIGH);
  digitalWrite(LEFT_SIGNAL_INPUT, HIGH);
  digitalWrite(BRAKE_INPUT, HIGH);

  mode = EEPROM.read(modeMemoryAddress);
}

// II. The loop routine runs over and over again forever(or at least as long as the board has power and is working...) 
//    This handles all of the actual light blinking and what not.
void loop() 
{
  // 1. First the program will check if the brakes are on; if all inputs are 1, then nothing is on.  
  //    So nothing is lit up, set all the lights to LOW which is off.
  if((digitalRead(RIGHT_SIGNAL_INPUT) == 1) and (digitalRead(LEFT_SIGNAL_INPUT) == 1) and (digitalRead(BRAKE_INPUT) == 1) and (!isRightTap) and (!isLeftTap) and (!isEmergencyTap) )
  { 
    off();
    resetFlashCount();
  } 
   
   // 2. Next check if the brakes are on (when BRAKE_INPUT is 0).
   if(digitalRead(BRAKE_INPUT) == 0)
   {
     if((digitalRead(RIGHT_SIGNAL_INPUT) == 1) and (digitalRead(LEFT_SIGNAL_INPUT) == 1) and (digitalRead(BRAKE_INPUT) == 0) and mode == 0)
     { 
       brakeSequenceFlashOn(FAST_BLINK);
     }
    
     // 2.1 Keep the lights lit as long as the brake is on 
     while(digitalRead(BRAKE_INPUT) == 0)
     {         
      
      if((digitalRead(LEFT_SIGNAL_INPUT) == 1) and (digitalRead(RIGHT_SIGNAL_INPUT) == 1))
      {
        (mode == 0) ? (brakeSequenceHoldOn()) : (brake());
      }
       
      // 2.2 Here is if the right turn signal and brake are on.
      if( (digitalRead(LEFT_SIGNAL_INPUT) == 1) and ( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) || (isRightTap) ) )
      {            
        (mode == 0) ? (rightSequenceBrake(QUICK_BLINK, QUICK_BLINK)) : (rightBlinkBrake(SLOW_BLINK, MEDIUM_BLINK));
      }
      
      // 2.3. Now if the left turn signal and brake are both on
      if( (digitalRead(RIGHT_SIGNAL_INPUT) == 1) and ( (digitalRead(LEFT_SIGNAL_INPUT) == 0) || (isLeftTap) ) )
      {
        (mode == 0) ? (leftSequenceBrake(QUICK_BLINK, QUICK_BLINK)) : (leftBlinkBrake(SLOW_BLINK, MEDIUM_BLINK));               
      }
     } 

     if((digitalRead(LEFT_SIGNAL_INPUT) == 1) and (digitalRead(RIGHT_SIGNAL_INPUT) == 1) and (mode == 0) and (!isRightTap) and (!isLeftTap) )
     {
       brakeSequenceFlashOff(FAST_BLINK);   
     }
   }
   
  // 3. Now checking for the emergency flashers being on, when this happens a fun litle flashing pattern is performed to warn other drivers of emergencies. If tapped the mode will change 
  else if(( digitalRead(RIGHT_SIGNAL_INPUT) == 0) and (digitalRead(LEFT_SIGNAL_INPUT) == 0) and (digitalRead(BRAKE_INPUT) == 1) || (isEmergencyTap) )
  {
    (mode == 0) ? (emergencyFlasherSequence(FAST_BLINK, MEDIUM_BLINK, SLOW_BLINK)) : (emergencyFlasherBlink(SLOW_BLINK, MEDIUM_BLINK));    
  }

  // 4. This is the section if the brakes are on and the emergency flashers are on. The brakes will light in this case. 
  if(( digitalRead(RIGHT_SIGNAL_INPUT) == 0) and (digitalRead(LEFT_SIGNAL_INPUT) == 0) and (digitalRead(BRAKE_INPUT) == 0))
  {
    if(mode == 0)
      brakeSequenceFlashOn(FAST_BLINK);
    
    while(digitalRead(BRAKE_INPUT) == 0)
    {         
      (mode == 0) ? (brakeSequenceHoldOn()) : (brake()); 
    }
    
    if(mode == 0)
      brakeSequenceFlashOff(FAST_BLINK);
  } 
  
  else
  {
    // 5. This is if the right turn signal is on, do a cycle through the lights.
    if( ( (digitalRead(LEFT_SIGNAL_INPUT) == 1) and (digitalRead(BRAKE_INPUT) == 1) ) and ( (digitalRead(RIGHT_SIGNAL_INPUT) == 0) || (isRightTap) ) )
    {
      (mode == 0) ? (rightSequence(QUICK_BLINK, QUICK_BLINK)) : (rightBlink(SLOW_BLINK, MEDIUM_BLINK));
    }
    
    // 6. Here is if the left turn signal is on. Cycle through the lights.
    if( ( (digitalRead(RIGHT_SIGNAL_INPUT) == 1) and (digitalRead(BRAKE_INPUT) == 1) ) and ( (digitalRead(LEFT_SIGNAL_INPUT) == 0) || (isLeftTap) ) )
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
  
  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(ON, OFF, OFF, OFF);
  delay(durationMillis);
  
  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(ON, ON, OFF, ON);
  delay(durationMillis);
  
  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(ON, ON, ON, ON);
  delay(durationMillis);

  off();
  delay(pauseDurationMillis);

  updateTapControl();
}

void rightBlink(int durationMillis, int offDurationMillis)
{
  checkIsTap(); 
  
  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(OFF, ON, ON, ON);
  delay(durationMillis);
  
  off();
  delay(offDurationMillis);

  updateTapControl();
}

void leftSequence(int durationMillis, int pauseDurationMillis)
{
  checkIsTap(); 
  
  selectLeftOn(ON, OFF, OFF, OFF);
  selectRightOn(OFF, OFF, OFF, OFF);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, OFF, ON);
  selectRightOn(OFF, OFF, OFF, OFF);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, ON, ON);
  selectRightOn(OFF, OFF, OFF, OFF);
  delay(durationMillis);

  off();
  delay(pauseDurationMillis);

  updateTapControl();
}

void leftBlink(int onDurationMillis, int offDurationMillis)
{
  checkIsTap(); 
  
  selectLeftOn(OFF, ON, ON, ON);
  selectRightOn(OFF, OFF, OFF, OFF);
  delay(onDurationMillis);
  
  off();
  delay(offDurationMillis);

  updateTapControl();
}

void rightSequenceBrake(int durationMillis, int pauseDurationMillis)
{
  checkIsTap(); 
  
  selectRightOn(ON, OFF, OFF, OFF);
  selectLeftOn(ON, ON, ON, OFF);
  delay(durationMillis);
  
  selectRightOn(ON, ON, OFF, ON);
  selectLeftOn(ON, ON, ON, OFF);
  delay(durationMillis);
  
  selectRightOn(ON, ON, ON, ON);
  selectLeftOn(ON, ON, ON, OFF);
  delay(durationMillis);

  selectRightOn(OFF, OFF, OFF, OFF);
  selectLeftOn(ON, ON, ON, OFF);
  delay(pauseDurationMillis);

  updateTapControl();
}

void rightBlinkBrake(int onDurationMillis, int offDurationMillis)
{
  checkIsTap(); 
  
  selectLeftOn(OFF, ON, ON, OFF);
  selectRightOn(OFF, ON, ON, ON);
  delay(onDurationMillis);
  
  selectLeftOn(OFF, ON, ON, OFF);
  selectRightOn(OFF, OFF, OFF, OFF);
  delay(offDurationMillis);
  
  updateTapControl();
}

void leftSequenceBrake(int durationMillis, int pauseDurationMillis)
{
  checkIsTap(); 
  
  selectLeftOn(ON, OFF, OFF, OFF);
  selectRightOn(ON, ON, ON, OFF);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, OFF, ON);
  selectRightOn(ON, ON, ON, OFF);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, ON, ON);
  selectRightOn(ON, ON, ON, OFF);
  delay(durationMillis);

  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(ON, ON, ON, OFF);
  delay(pauseDurationMillis);
  
  updateTapControl();
}

void leftBlinkBrake(int onDurationMillis, int offDurationMillis)
{
  checkIsTap(); 
  
  selectLeftOn(OFF, ON, ON, ON);
  selectRightOn(OFF, ON, ON, OFF);
  delay(onDurationMillis);

  selectLeftOn(OFF, OFF, OFF, OFF);
  selectRightOn(OFF, ON, ON, OFF);
  delay(offDurationMillis);
  
  updateTapControl();
}

void brakeSequenceFlashOn(int durationMillis)
{
  selectLeftOn(ON, OFF, OFF, OFF);
  selectRightOn(ON, OFF, OFF, OFF);
  delay(durationMillis);
  
  selectLeftOn(ON, ON, OFF, OFF);
  selectRightOn(ON, ON, OFF, OFF);
  delay(durationMillis);
}

void brakeSequenceFlashOff(int durationMillis)
{
  selectLeftOn(ON, ON, OFF, OFF);
  selectRightOn(ON, ON, OFF, OFF);
  delay(durationMillis);

  selectLeftOn(ON, OFF, OFF, OFF);
  selectRightOn(ON, OFF, OFF, OFF);
  delay(durationMillis);
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
  
  brakeSequenceFlashOn(flashDurationMillis);  
  selectLeftOn(ON, ON, ON, ON);
  selectRightOn(ON, ON, ON, ON);
  delay(onDurationMillis);    
  
  brakeSequenceFlashOff(flashDurationMillis); 
  off();
  delay(offDurationMillis);

  updateTapControl();
}

void emergencyFlasherBlink(int onDurationMillis, int offDurationMillis)
{
  checkIsTap();
  
  selectLeftOn(OFF, ON, ON, ON);
  selectRightOn(OFF, ON, ON, ON);
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

