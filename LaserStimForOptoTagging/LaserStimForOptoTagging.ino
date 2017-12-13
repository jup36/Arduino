/*
Spontaneous stimulation for opto-tagging.
Activate stim avoiding time of joystick movement.  

*/

#define LASER     3  // digital pin 3 currently connected to a BNC channel in BCS

//=======================
// Set the protocol name
char protocolName[] = "SpontaneousOptoTagging"; // should be less than 20 characters
unsigned long beginLoopTime;
unsigned long loopTime;
//=======================

//=======================
// set pin numbers for relevant inputs and outputs:
int analogPins[] = {0,1,2,3};
//=======================

//=======================
// Arrays that will hold the current values of the AO and DIO
int CurrAIValue[] = {0,0,0,0};
int aiThresh[] =  {512,512,512,512};
int bufferLength = 100;
int bufferEndIndex = bufferLength-1;
int frozen=0;
int first=0;
int bufferAI0[100];
int bufferAI1[100];
int bufferAI2[100];
int bufferAI3[100];
//=======================

//=======================
// PARAMETERS OF TASK
unsigned long interStimInterval = 10000; // inter-stimulation interval, default = 10000 ms 
unsigned long stimDuration = 500;        // stim-duration, default = 500 ms
int xCenter = 0;
int xWidth = 10;
int yCenter = 0;
int yWidth = 10;
//=======================

//=======================
// VARIABLES OF TASK
unsigned long time;           // all values that will do math with time need to be unsigned long datatype
unsigned long stimTime = 0;   // LASER onset time
unsigned long trialEnd = 0;
unsigned long trialStart = 0;
int ParadigmMode = 0;         
int jsZeroX = 512;
int jsZeroY = 512;
int xDisp = 0;
int yDisp = 0;
int prevXDisp = 0;
int prevYDisp = 0;
int xDispS = 0;
int yDispS = 0;
int count = 0;
boolean success = false;
boolean stimActive = false; // logic to control stim active or not
//=======================

//==========================================================================================================================
// SETUP LOOP
//==========================================================================================================================
void setup() {
  
      //=======================    
        pinMode(LASER, OUTPUT);
        digitalWrite(LASER, LOW);
      //=======================

      //=======================
      // prep ANALOG inputs
        analogReference(DEFAULT);
      //=======================
    
      //=======================
      // initialize zero positions
       jsZeroX = analogRead(0);
       jsZeroY = analogRead(1);
      //=======================

      //=======================
      // initialize the SERIAL communication
        Serial.begin(115200);
      //=======================

      count = 0; // initialize the stimulation counter
      
      //=======================
      // initialize analog read buffer
      for (int j=0;j<bufferLength;j++) {
        bufferAI0[j] = 0;
        bufferAI1[j] = 0;
        bufferAI2[j] = 0;
        bufferAI3[j] = 0;        
      }

      //=======================
      
      CurrAIValue[0] = analogRead(0); // x position
      CurrAIValue[1] = analogRead(1); // y position
      CurrAIValue[2] = analogRead(2);
      CurrAIValue[3] = analogRead(3);  
}


//==========================================================================================================================
// MAIN EXECUTION LOOP
//==========================================================================================================================
void loop() {
  
      beginLoopTime = micros();
      time = millis();         

      // Left shift the buffer
      for (int i=1;i<bufferLength;i++) { // i<bufferLength
          bufferAI0[i-1] = bufferAI0[i];
          bufferAI1[i-1] = bufferAI1[i];
      }

      bufferAI0[bufferEndIndex] = int(analogRead(0)); // joystick x ... takes 110us
      bufferAI1[bufferEndIndex] = int(analogRead(1)); // joystick y
          
      CurrAIValue[0] = bufferAI0[bufferEndIndex]; 
      CurrAIValue[1] = bufferAI1[bufferEndIndex];
    
      switch (ParadigmMode) { // ParadigmMode must have been initialized to 0 
      
        case 0: // just take the zero position and go to the next case

            jsZeroX = CurrAIValue[0]; // X0
            jsZeroY = CurrAIValue[1]; // Y0
            
            if (count==0) { // for the first trial 
              stimActive = true;  // activate the stim logic 
              ParadigmMode = 1;   // just go to the stim block and see if stim can be delivered 
            } else {
              if (time > stimTime + interStimInterval) {
                ParadigmMode = 1; // go to the stim block once interStimInterval elapsed since the last stim trial
              }
            }
        break;
        
        case 1: // STIM BLOCK; look for lever threshold crossings and open valve in response

             xDisp = abs(CurrAIValue[0]-jsZeroX);     // find X displacement of sample in relation to start of reach
             yDisp = abs(CurrAIValue[1]-jsZeroY);     // find Y displacement of sample in relation to start of reach

             if (xDisp<10 && yDisp<10 && stimActive==true) {   // if the joystick movement is negligible (less than a set low threshold; e.g. 10)
                digitalWrite(LASER, HIGH); // stimulate; LASER ON
                stimTime = time;           // mark the stim time
                ParadigmMode = 2;          // go to stim end block
                stimActive = false;        // inactivate the stim logic to prevent continuous stimulation
             }          
        break;
                     
        case 2: // TRIAL END BLOCK; 
                
                ParadigmMode = 0;
                count++;
                Serial.print(count); // to monitor the number of stimulation given through the serial monitor
        break;
      }
           
      // just loop until 1ms has elapsed without locking up the processor
      while(loopTime<1000 && loopTime>0) {
        loopTime = micros() - (beginLoopTime*1000);
      }
      loopTime = 1;    
     
      // turn the LASER STIM OFF
      if (time > stimTime+stimDuration) { // turn the laser off after stimDuration has elapsed (laser stim duration: 500 ms) 
        digitalWrite(LASER, LOW);         // stim for stimDuration length (currently 450 ms)
        stimTime = 0; 
      }

      if (time > stimTime+interStimInterval && count < 10){ // this also is to prevent continuous stim, but it also allows to stim again after isi elapses
        stimActive = true;
      }   

} // END OF LOOP

