/*
Serial codes are currently being modified to allow for the host computer to control the initiation and termination of trials.
*/

void serialEvent(Serial p) {
  serialEventLogic = true;
}

//========================================
// CONTROLLER FOR SENDING OUT THE SERIAL CODES
void sendSerialCode(int type) {

  switch(type) {

    case 1: // REQUEST DATA
      port.write(87);
      break;
      
    case 2: // BEGIN A TRIAL
      // MUST BE A SINGLE BYTE (-127->128) FOR THIS TRANSFER SCHEME
      //port.clear();
      port.write(90);
      port.write(0);            // available for future use
      port.write(boundaryWidth);// available for future use
      port.write(paramWave[2]); // valveOpenTimeX
      port.write(paramWave[3]); // xCenter
      port.write(paramWave[4]); // xWidth
      port.write(paramWave[5]); // yCenter
      port.write(paramWave[6]); // yWidth
      port.write(paramWave[7]); // valveOpenTimeY
      port.write(responseMode); // responseMode
      port.write(int(sqrt(valveDelayTime)));
      port.write(rightleft);
      port.write(1);
      println("Trial Start...");
      break;
      
    case 3: // EMERGENCY BREAK
      println("STOP");
      //port.clear();
      port.write(89);
      isRunning = 0;
      break;

    case 4: // REQUEST END OF TRIAL EVENT
      println("Request event");
      //port.clear();
      port.write(88);
      break;
    
    case 5: // RETRIEVE CURRENT BURNED CODE DETAILS
      println("Request name of protcol");
      //port.clear();
      port.write(91);
      break;
      
    case 6: // DIRECTLY ASSERT PIN VALUES & STATES
      pinNumber = 22;
      pinLogic = 1;
      println("Set pin: "+pinNumber+" to state: "+pinLogic);      
      port.clear();
      port.write(92);
      port.write(pinNumber);
      port.write(pinLogic);
      delay(100);
      pinLogic = 0;
      println("Set pin: "+pinNumber+" to state: "+pinLogic);      
      port.clear();
      port.write(92);
      port.write(pinNumber);
      port.write(pinLogic);
      break;
  }  
  
}
//========================================

//========================================
// GATHER DATA FROM INCOMING SERIAL DATA
void parseSerialData() {

    String input = port.readStringUntil('*');
     //println(input);  if you ever need to debug
     
    if (input != null) {
      int[] serialVals = int(split(input, ","));
      String[] parameters = split(input, ',');
      
      switch (serialVals[0]) {
        case 1:
          varA = serialVals[1]; // x displacement
          varB = serialVals[2]; // y displacement
          varC = serialVals[3]; // current velocity, currVelocity = int(sqrt((CurrAIValue[0]-prevXDisp)^2 + (CurrAIValue[1]-prevYDisp)^2));
          varD = serialVals[4]; // cross velocity, velocity at crossing of the threshold  
    
          digA = serialVals[5];
          //digB = serialVals[6];
          //digC = serialVals[7];
          //digD = serialVals[8];
          //digE = serialVals[9];
          //digF = serialVals[10];
          //digG = serialVals[11];
          //digH = serialVals[12]; 
          //digI = serialVals[13]; 
          //digJ = serialVals[14]; 
          //digK = serialVals[15]; 
          //digL = serialVals[16]; 
          //digM = serialVals[17]; 
          break;
                  
        case 3:
          if (time>lastEvent+500) {
            println("===========BOARD IS STOPPED===========");
            isRunning = 0;
          }
          break;
          
        case 4: // trial end data serial communication
          trialData[0] = serialVals[1]; // count
          trialData[1] = serialVals[2]; // trialStart
          trialData[2] = serialVals[3]; // crossTime
          trialData[3] = serialVals[4]; // trialEnd
          trialData[4] = serialVals[5]; // firstLickTime
          trialData[5] = serialVals[6]; // valveOpenTime
          trialData[6] = serialVals[7]; // crossVelocity; reach velocity measured at threshold crossing
          controlCode = 1;
          betweenTrials = 1;
          break;
          
        case 5:
          // Finally, must request the rest of the trial information
          //port.write(88);
          controlCode=3;
          break;
          
        case 6:
          println(parameters[0]);
          protName.setText(parameters[0]);
          break;
      }
            
    }
}