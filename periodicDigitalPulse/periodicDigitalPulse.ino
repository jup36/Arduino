/*
  Water spout training
*/

// define digital pins with input/output
#define SOLENOID 22 // output

// PARAMETERS OF TASK
unsigned long time;          // running calulation of time of program
unsigned long loopTime;      // monitors to make sure Arduino isn't sampling at a rate higher than 1 ms
unsigned long beginLoopTime; // 
long valveOpenDur = 75;   // duration for solenoid activation
long interTrialInterval = 10000;  // ITI
long lastTrialCompletion = 0;    // Time for last trial completion
int numbTrials = 40;      // number of trials
int trialCount = 0;       // count the number of trials

// VARIABLES OF TASK
long valveOpenTime = 0;  // valveopentiming
long valveCloseTime = 0; // valveclosetiming
int ParadigmMode = 0;    // task state control


void setup() {
  pinMode(SOLENOID, OUTPUT);  // initialize solenoid as an output pin
}


void loop() {

  beginLoopTime = micros();
  time = millis(); // sample current time in ms

  switch (ParadigmMode) {

  case 0: // check trial count
      
      if (trialCount < numbTrials)
        ParadigmMode = 1;
      break;
  
  case 1: // valve opens

      if ( time > (lastTrialCompletion + interTrialInterval) ) {
        
        digitalWrite(SOLENOID, HIGH); // valve opens
        valveOpenTime = time; // mark valve open time
        ParadigmMode = 2; // goes to case 1
      }

      break;
  
  case 2: // valve closes

      if ( time > (lastTrialCompletion + interTrialInterval + valveOpenDur) ) {  
  
        digitalWrite(SOLENOID, LOW);  // valve closes    
        lastTrialCompletion = time; 
        trialCount = trialCount + 1;
        ParadigmMode = 0;        
      
    }  
      
      break;
  }

  loopTime = 0;
  while (loopTime < 1000 && loopTime > 0) {  // this ensures that Arduino isn't sampling at a rate higher than 1000 Hz
    loopTime = micros() - beginLoopTime;
  }
  loopTime = 1;  
      
} // end of loop
