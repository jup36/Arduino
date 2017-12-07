/*
General functions for the control of code based serial communication with the Arduino board
*/

//=======================
void RunSerialCom(int code) {
       
  switch (code) {
    
    case 87: // data has been requested
      // Current settings allow for 4 8-bit digital lines ("AO") and 12 1-bit digital lines ("DO")
      Serial.print(1);
      Serial.print(",");
      Serial.print(CurrAIValue[0]-jsZeroX);
      Serial.print(",");
      Serial.print(CurrAIValue[1]-jsZeroY);
      Serial.print(",");
      Serial.print(CurrAIValue[2]);
      Serial.print(",");
      Serial.print(CurrAIValue[3]);
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[0]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[1]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[2]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[3]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[4]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[5]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[6]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[7]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[8]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[9]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[10]));
      Serial.print(",");
      Serial.print(digitalRead(digitalPins[11]));
      Serial.print(",");
      Serial.print("*");
      break;
      
    case 88: // trail ended and the trial details were requested              
      // tell the monitor about the trial details
      Serial.print(4);
      Serial.print(",");
      Serial.print(count);
      Serial.print(",");
      Serial.print(trialStart);      
      Serial.print(",");
      Serial.print(crossTime);      
      Serial.print(",");
      Serial.print(trialEnd);      
      Serial.print(",");
      Serial.print(firstLickTime);      
      Serial.print(",");
      Serial.print(valveOpenTime);      
      Serial.print(",");
      Serial.print(leftTrial);      
      Serial.print(",");
      Serial.print("*");
      break;

    case 90: // start trial (i.e. need to read from the serial port)
      delay(10);
      placeHolder = Serial.read();
      delay(10);
      placeHolder = Serial.read();
      delay(10);
      valveOpenTimeX = Serial.read();
      delay(10);
      xCenter = Serial.read();
      delay(10);
      xWidth = Serial.read();
      delay(10);
      yCenter = Serial.read();
      delay(10);
      yWidth = Serial.read();
      delay(10);
      valveOpenTimeY = Serial.read(); // switch to be byte wise
      delay(10);
      responseMode = Serial.read();
      delay(10);
      valveDelayTmp = Serial.read();
      delay(10);
      leftTrial = Serial.read();      
      
      valveDelayTime = valveDelayTmp*valveDelayTmp;
            
      digitalWrite(digitalPins[0], LOW); // trial cue light
      digitalWrite(digitalPins[1], LOW); // success event
//      digitalWrite(digitalPins[2], LOW); // performance cue light
      digitalWrite(digitalPins[3], LOW); // valve
      digitalWrite(digitalPins[4], LOW); // reset the lick indicator 

      // set the ParadigmMode to move into a trial
      ParadigmMode = 1;

      break;
      
    case 91:
      Serial.print(6);
      Serial.print(",");
      Serial.print(protocolName);      
      Serial.print(",");
      Serial.print("*");
      break;
      
    case 92: // directly control the arduino pin settings
      // format is: read the code and if 92 read the next few values for updating of parameters from the BehaviorMonitor
      testMode=1;
      
      delay(10);
      assertPinNum = Serial.read();
      delay(10);
      assertPinState = Serial.read();
      delay(10);
      
      forcePin(assertPinNum,assertPinState);
      
      break;

  }
  
}
//=======================

//=======================
void forcePin(int assertPinNum, int assertPinState) {
  
  if (ParadigmMode==0) {
    if (assertPinState==0) {
      digitalWrite(assertPinNum, LOW);
    } else {
      digitalWrite(assertPinNum, HIGH);
    }
  }
}
//=======================

