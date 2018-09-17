/*
THIS IS A BRANCH TO INTRODUCE A CONSTANT DIGITAL PULSES FOR STIM-THRESHOLD CROSSING. 
fixLeverOp
Behavioral task waits for a serial command to begin trials and looks for some movement of a joystick to trigger
an output response.
Modified on Dec/17 to incorporate generation of digital pulses for laser stimulation on randomly selected trials at user defined percentage. 
*/

//// include the library code:
//#include <SPI.h>
//// pick up the SPI pins from the arduino header file
//#include <pins_arduino.h>

#define BEAM1     36
#define SOLENOID1 22
#define ADC_PIN   49
#define DAC_PIN   48
#define LASER     3    // signals laser delivery
#define pseudoLaser 21 // signals pseudoLaser delivery (crossing of the laser trigger threshold)

//=======================
// Set the protocol name
char protocolName[] = "FixLever_v10_LL"; // should be less than 20 characters
unsigned long beginLoopTime;
unsigned long loopTime;
//=======================

//=======================
// set pin numbers for relevant inputs and outputs:
int analogPins[] = {0,1,2,3};
int digitalPins[] = {4,5,6,7,8,41,43,45,36,38,39,40}; // added digitalPin 3 that is connected/valid on the BCS to trigger laser (12/7)
//=======================

//=======================
// Arrays that will hold the current values of the AO and DIO
int CurrAIValue[] = {0,0,0,0};
int aiThresh[] =  {512,512,512,512};
int bufferLength = 5;
int bufferEndIndex = bufferLength-1;
int frozen=0;
int first=0;
int bufferAI0[100];
int bufferAI1[100];
int bufferAI2[100];
int bufferAI3[100];
int bufferLoc = 0;
//=======================

//=======================
// PARAMETERS OF TASK
unsigned long interTrialInterval = 500;//15000;
unsigned long stimDelay = 0;//500;
unsigned long stimDuration = 0;//500;
int stimTrial = 0;  // laser stim logic; 0: stim OFF, 1: stim ON
boolean stimActive = false; // internal (not serial communicated) logic to prevent continuous stim. 
unsigned long valveOpenTime = 15;
unsigned long valveOpenTimeX = 30;
unsigned long valveOpenTimeY = 30;
unsigned long valveDelayTime = 400;//1000;
int placeHolder = 0;
int xCenter = 0;
int xWidth = 10;
int yCenter = 0;
int yWidth = 10;
int responseMode = 0;
int leftTrial = 0;
int valveDelayTmp = 0;
int cntX = 0;
int cntY = 0;

//=======================

//=======================
// VARIABLES OF TASK
unsigned long time; // all values that will do math with time need to be unsigned long datatype
unsigned long crossTime = 0;
unsigned long rewardDelivered = 0;
unsigned long stimTime = 0; // LASER onset time
boolean firstLickLogic = false;
unsigned long trialEnd = 0;
unsigned long trialStart = 0;
unsigned long firstLickTime = 0;
int ParadigmMode = 0;
int testMode = 0;
int jsZeroX = 512;
int jsZeroY = 512;
int xDisp = 0;
int yDisp = 0;
int prevXDisp = 0;
int prevYDisp = 0;
int xDispS = 0;
int yDispS = 0;
int count = 0;
long displacement = 0;
boolean success = false; // signal whether reach should be rewarded 
//=======================

//=======================
// PARAMETERS OF HARDWARE
int lThresh = 300;
//=======================

//=======================
// VARIABLES FOR SERIAL COM
int code = 0;
int assertPinNum = 0;
int assertPinState = 0;
//=======================

//==========================================================================================================================
// SETUP LOOP
//==========================================================================================================================
void setup() {
  
      //=======================
      // prep DIGITAL outs and ins
        pinMode(digitalPins[0], OUTPUT);
        pinMode(digitalPins[1], OUTPUT);
        pinMode(digitalPins[2], OUTPUT);
        pinMode(digitalPins[3], OUTPUT);
        pinMode(digitalPins[4], OUTPUT);
        pinMode(digitalPins[5], OUTPUT);
        pinMode(digitalPins[6], OUTPUT);
        pinMode(digitalPins[7], OUTPUT);
        digitalWrite(digitalPins[0], LOW);
        digitalWrite(digitalPins[1], LOW);
        digitalWrite(digitalPins[2], LOW);
        digitalWrite(digitalPins[3], LOW);        
        digitalWrite(digitalPins[4], LOW);        
        digitalWrite(digitalPins[5], LOW);        
        digitalWrite(digitalPins[6], LOW);        
        digitalWrite(digitalPins[7], LOW);        
      //=======================

      //=======================
      // two logic pins set high for testing digital signals
        pinMode(26, OUTPUT);
        pinMode(28, OUTPUT);
        digitalWrite(26, HIGH);
        digitalWrite(28, HIGH);     
        pinMode(SOLENOID1, OUTPUT);
        digitalWrite(SOLENOID1, LOW);
        pinMode(LASER, OUTPUT);
        digitalWrite(LASER, LOW);
        pinMode(pseudoLaser, OUTPUT);
        digitalWrite(pseudoLaser, LOW);
      //=======================

      //=======================
      // prep ANALOG inputs
        analogReference(DEFAULT);
      //=======================
      
      //=======================
      // initialize the SERIAL communication
        Serial.begin(115200);
      //=======================
      
//      //=======================
//      // prep SPI for AD control
//      startSPI();
//      //=======================
      
  
    //=======================
    // initialize SERIAL for LCD
      Serial1.begin(19200); 
      Serial1.write(byte(0x0c));   //clear display
      delay(5);
      Serial1.write(byte(0x11));   // Back-light on
      Serial1.write(byte(0x80));   // col 0, row 0
      Serial1.print(protocolName); // print protocol name
    //=======================
  
    //=======================
    // initialize zero positions
       jsZeroX = analogRead(0);
       jsZeroY = analogRead(1);
    //=======================

      count = 0; // initialize the trial counter
      
      //=======================
      // initialize analog read buffer
      for (int j=0;j<bufferLength;j++) {
        bufferAI0[j] = 0;
        bufferAI1[j] = 0;
        bufferAI2[j] = 0;
        bufferAI3[j] = 0;        
      }
      bufferLoc = 0;
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

          bufferAI0[bufferEndIndex] = int(analogRead(0)-jsZeroX); // joystick x ... takes 110us
          bufferAI1[bufferEndIndex] = int(analogRead(1)-jsZeroY); // joystick y

          cntX = 0; // 
          cntY = 0; // 

          CurrAIValue[0] = bufferAI0[bufferEndIndex]; // gather new current x position
          CurrAIValue[1] = bufferAI1[bufferEndIndex]; // gather new current y position
          CurrAIValue[3] = digitalRead(BEAM1);  // lick port

       if(CurrAIValue[3]<lThresh) {
         digitalWrite(digitalPins[4], HIGH); // signal a lick
         if(firstLickLogic) {
          firstLickTime = time;
          firstLickLogic = false; 
         }
       }    
        

      switch (ParadigmMode) {
      
        case 0: // just idle in this state waiting for controller to start next trial
            Serial1.write(byte(0x94));  // col 0, row 0
            Serial1.print("ITI...");
            Serial1.print(constrain(CurrAIValue[0],0,999));
            Serial1.print(".");
            Serial1.print(constrain(CurrAIValue[1],0,999));
            break;
            
        case 1: // if controller tells you to start, then start      

//            jsZeroX = CurrAIValue[0];
//            jsZeroY = CurrAIValue[1];
            trialStart = time;
            digitalWrite(digitalPins[0], HIGH); // trial cue light 4            
            digitalWrite(digitalPins[5], HIGH); // trial cue indicator to send to the recording system
            ParadigmMode = 2;
            Serial1.write(byte(0x0c));   // clear the display
            Serial1.write(byte(0x94));  
            Serial1.print("START");
            break;
        
//==========================================================================================================================
// RULES FOR EVENT TRIGGERS
//==========================================================================================================================
        case 2: // look for lever threshold crossings and open valve in response

            switch(responseMode) {     
              case 0:
                
//                xDispS = abs(prevXDisp-jsZeroX);         // find X displacement of previous samples in relation to start of reach
//                yDispS = abs(prevYDisp-jsZeroY);         // find Y displacement of previous samples in relation to start of reach

                xDisp = abs(CurrAIValue[0]);     // find X displacement of sample in relation to start of reach
                yDisp = abs(CurrAIValue[1]);     // find Y displacement of sample in relation to start of reach

                for (int i=0;i<bufferLength;i++) { // i<bufferLength
                  if (abs(bufferAI0[i])>xWidth) {
                    cntX++;
                  }
                  if (abs(bufferAI1[i])>yWidth) {
                    cntY++;
                  }
                }

                if (cntX>=bufferEndIndex || cntY>=bufferEndIndex) {  // if exceeds low velocity threshold and it is stimTrial 1                  
                  if (stimActive==true){ // if stim is not refractory
                   if (stimTrial==1){ // if current trial is a stim trial
                    digitalWrite(LASER, HIGH); // stimulate; LASER ON
                    stimTime = time;    // mark stim activation time
                    stimActive = false; // make stim inactive to prevent continuous stim. 
                    stimTrial=0; // this is to prevent repetitive stim. 
                    //ParadigmMode = 1;   // returning back to paradigmmode 1 would prevent repeating laser stim for static low displacement            
                   } else { // if not a stim trial
                    digitalWrite(pseudoLaser, HIGH);     // trigger a low-threshold crossing signal
                    stimTime = time;    // mark stim activation time 
                    stimActive = false; // make stim inactive to prevent continuous signaling. 
                    //ParadigmMode = 1;   // returning back to paradigmmode 1 would prevent repeating laser stim for static low displacement                   
                   }
                  }
                }
                 
               if (cntX>=bufferEndIndex || cntY>=bufferEndIndex) {         // if moving out from center (make sure current sample is larger than previous)
                   crossTime = time;                             // mark cross time to check if the next buff samples go above upper threshold
                   //success   = true;                           // for the time being, success = true

                   frozen = 0;        
                   valveOpenTime = valveOpenTimeY;
                   firstLickLogic = true;
                   digitalWrite(digitalPins[1], HIGH); // success event pulse
                   digitalWrite(digitalPins[0], LOW);  // trial cue 
                   digitalWrite(digitalPins[6], HIGH); // performance cue indicator to send to the recording system
                   ParadigmMode = 3;
                   Serial1.write(byte(0x0c));   // clear the display
                   Serial1.write(byte(0x94));  
                   Serial1.print("EVENT");
                   //success = false;      // reset success to false
                }
          
                break;
           
            }
            
            break;
//==========================================================================================================================
// RULES FOR EVENT TRIGGERS
//==========================================================================================================================
    
        case 3:
            
            if (time > (crossTime+250) ) {
              analogWrite(digitalPins[2], 0);              
            }
            
            if(CurrAIValue[3]<lThresh && first==0) {
              trialEnd = time;
              first = 1;
            }
            
            if(time > (crossTime+valveDelayTime) ) {
              digitalWrite(SOLENOID1, HIGH);      // valve opens
              rewardDelivered = time;
              digitalWrite(digitalPins[7], HIGH); // trial cue indicator to send to the recording system
              ParadigmMode = 4;              
              Serial1.write(byte(0x0c));      // clear the display
              Serial1.write(byte(0x94));  
              Serial1.print("REWARD");
            }
            
            break;
    
        case 4: // close valve once the reward has had sufficient time to be delivered
            
            if(CurrAIValue[3]<lThresh && first==0) {
              trialEnd = time;
              first = 1;
            }
            
            if(valveOpenTime<10) {
              valveOpenTime=10;
            }
            
            
            if(time > (crossTime+valveDelayTime+valveOpenTime) ) {
              digitalWrite(SOLENOID1, LOW);  // valve closes
              ParadigmMode = 5;              
              Serial1.write(byte(0x0c));   // clear the display
              Serial1.write(byte(0x94));  
              Serial1.print("POST-REWARD");
            }      

            break;
              
        case 5: // this is the end of trial state

            if(CurrAIValue[3]<lThresh && first==0) {
              trialEnd = time;
              first = 1;
            }
            
            if ( time-rewardDelivered>3000 ) { // this must keep iti
              
              if (first==0) {
                trialEnd = 3000+crossTime;
              }

              // send a notice of trial end
              Serial.print(5);
              Serial.print(",");
              Serial.print("*");
              
              // defrost the recording buffer
              frozen = 0;
              first = 0;
              
              stimActive = true; // convert stimActive back to true to let stim possible in the next trial
              
              ParadigmMode = 0;
             Serial1.write(byte(0x0c));   // clear the display
             Serial1.write(byte(0x94));  
              count++;
              
            }
            break;
    
      }
           
      if (Serial.available() > 0) {

        code = Serial.read();

        switch (code) {
        
          case 89: // stop execution
            ParadigmMode = 0;
            Serial.print(3);
            Serial.print(",");     
            Serial.print("*");
           
            digitalWrite(digitalPins[0], LOW); // success event
            digitalWrite(digitalPins[1], LOW); // trial cue light
//            digitalWrite(digitalPins[2], LOW); // performance cue light
            digitalWrite(digitalPins[3], LOW); // valve
            digitalWrite(digitalPins[4], LOW); // reset the lick indicator
            break;
            
        // all other codes are handshaking codes either requesting data or sending data 
          case 87:
            RunSerialCom(code); 
            break;
    
          case 88:
            RunSerialCom(code); 
            break;

          case 90:
            RunSerialCom(code); 
            break;
    
          case 91:
            RunSerialCom(code);
            break;
    
          case 92:
            RunSerialCom(code);
            break;
            
        }
        
        Serial.flush();
    
      }
    
    
    // pause until 1ms has elapsed without locking up the processor
      while(loopTime<1000 && loopTime>0) {
        loopTime = micros() - (beginLoopTime*1000);
      }
      loopTime = 1;    
      
//    THESE SHOULD JUST BE CONTROLLED BY PROTOCOL
//    digitalWrite(digitalPins[0], LOW); // trial cue light
//    digitalWrite(digitalPins[2], LOW); // performance cue light
//    digitalWrite(digitalPins[3], LOW); // valve 
    digitalWrite(digitalPins[1], LOW); // success event
    digitalWrite(digitalPins[4], LOW); // reset the lick indicator
    digitalWrite(digitalPins[5], LOW); // 
    digitalWrite(digitalPins[6], LOW); // 
    digitalWrite(digitalPins[7], LOW); // 

    // turn the LASER STIM OFF
    if (time > stimTime+450) {      // turn the laser off after 450 ms (laser stim duration: 450 ms) 
      digitalWrite(LASER, LOW);     // stim for stimDuration length (currently 450 ms)
      digitalWrite(pseudoLaser, LOW); // turn the pseudoLaser stim off
    }
    
    if (time > stimTime+5000){ // this also is to prevent continuous stim, but it also allows to stim again after 3 sec elapsed from the last stim onset
      stimActive = true;
      digitalWrite(LASER, LOW);     // stim for stimDuration length (currently 450 ms)
      digitalWrite(pseudoLaser, LOW); // turn the pseudoLaser stim off
    }
    
}
