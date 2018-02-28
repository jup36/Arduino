 
//===========================================
//===========================================
// parse the end of trial serial data
void eot_parser() {
  TrialCounter.setValue(trialData[0]);
  TrialStart.setValue(trialData[1]/1000);
  TrialEvent.setValue(trialData[2]/1000);
  TrialEnd.setValue(trialData[3]/1000);
  
  
  if(trialData[0]>129) {
    if(trialData[0]>257) {
      trialD[int(trialData[0])-257]=int(log(trialData[2]-trialData[1])*3); // will be approximately bounded at 3*12          
    } else {
      trialD[int(trialData[0])-129]=int(log(trialData[2]-trialData[1])*3); // will be approximately bounded at 3*12                
    }
  } else {
    trialD[int(trialData[0])]=int(log(trialData[2]-trialData[1])*3); // will be approximately bounded at 3*12
  }
  // STORE THE TRIAL DATA IN AN ASSOCIATED TEXT FILE
  String data = trialData[0] + "," + trialData[1] + "," + trialData[2] + "," + trialData[3] + "," + trialData[6]; // trialData[4] + "," + trialData[5] + "," + trialData[6]+ ","+ paramWave[8]+ ","+ variable + "," + rightleft;
  // trialData[0]: count (trial count)
  // trialData[1]: trialStart (trial start time)
  // trialData[2]: crossTime (time of threshold crossing)
  // trialData[3]: trialEnd (time trial ended)
  // trialData[6]: crossVelocity (reach velocity measured at threshold crossing)
  output.println(data);
  output.flush();
  
  // also need to store parameters of previous trial
  String dataP = trialData[0] + "," + paramWave[0] + "," + paramWave[1] + "," + paramWave[2] + "," + paramWave[3] + "," + paramWave[4] + "," + paramWave[5] + "," + paramWave[6] + "," + paramWave[7] + "," + paramWave[8] + "," + paramWave[9];
  parameters.println(dataP);
  parameters.flush();
  
  // store the end of the last trial
  lastTrialCompletion = time; // no local punishment for long delay trials
  
  if (endSessionLogic) {
    controlP5.controller("run").setColorBackground(0xff003652);
    testMode = 1;
    isRunning = 0;
    initialRun=true;
    println("Stopped");
    lastEvent = time;
    controlCode=4;
  } 
  else {
    controlCode=2;
  }
}


//===========================================
//===========================================
// this function is used to determine the parameters for the subsequent trial
void eot_parameter() {

  ParamDisp10.setValue(rightleft);
  
  responseMode = int(ParamDisp9.value());
  interTrialInterval = int(ParamDisp0.value());

  // set stimulus parameters
  stimDelay = 500;
  stimDuration = 500;

  valveOpenTime = int(ParamDisp7.value()); // should be set by user
  valveDelayTime = 1000;

  xCenter = int(ParamDisp3.value());
  yCenter = int(ParamDisp5.value());
  xWidth = int(ParamDisp4.value());
  yWidth = int(ParamDisp6.value());


  // SWITCH TO THE STAIRCASE PARADIGM
  blockType = floor(count/40); // each block comprises 40 trials by default
  
  if (blockType>=4) {  // terminate the session once four blocks completed 
    // session is complete
    println(blockType);
    println(count);    // trial count
    endSessionLogic = true;
  } else { // set params for the next trial
    println(blockType);
    println(count);
    yWidth = thresholds[blockType];
    xWidth = thresholds[blockType];
    
    float rand = random(100); // draw a random number (type float) within the range of [0 100] 
    if (rand <= stimPerc & stimSession == 1){ // stim logic for the next trial
    stimTrial = 1; // stim
    } else {
    stimTrial = 0; // no stim
    }
  
    ParamDisp4.setValue(xWidth);
    ParamDisp6.setValue(yWidth);
    
  }
  
  
// FOR TARGET LEVER ////////////////////
//   rightleft = int (random(1,3));   
//int blockwise = 1;
//int [] targetDirection = {2,1,2,1,2,1,2,2,1,2,1,2,1};

//if (blockwise == 1){
//  rightleft = targetDirection[blockType];
//}
  
  paramWave[0] = stimTrial; // stim trial logic
  paramWave[1] = stimDelay; // stim trial latency
  paramWave[2] = valveOpenTimeX;
  paramWave[3] = xCenter;
  paramWave[4] = xWidth;
  paramWave[5] = yCenter;
  paramWave[6] = yWidth;
  paramWave[7] = valveOpenTimeY;
  paramWave[8] = valveDelayTime;
  paramWave[9] = responseMode;
  
//  ParamDisp0.setValue(interTrialInterval);
  ParamDisp1.setValue(stimDelay);
  ParamDisp2.setValue(stimDuration);
//  ParamDisp3.setValue(xCenter);
//  ParamDisp4.setValue(xWidth);
//  ParamDisp5.setValue(yCenter);
//  ParamDisp6.setValue(yWidth);
//  ParamDisp7.setValue(valveOpenTime);
  ParamDisp8.setValue(valveDelayTime);
//  ParamDisp9.setValue(responseMode);
}

//===========================================
//===========================================
void eot_analysis() {
// this is a function to calculate any online analysis needed...

// at minimum need a trial counter; also would like a running average of latencies for example
  count++;
  
//  //===========================================
//  // SEND CURRENT INFO
//  transmittedPacket = str(trialData[1])+","+str(TrialCounter.value())+",*";
//  myServer.write(transmittedPacket);
//  //===========================================

}

int GenerateGaussianDelay(float variance, float mean) {

  float x = 0;
  float y = 0;
  float s = 2;

  while (s>1) {
    x=random(-1,1);
    y=random(-1,1);
    s = (x*x) + (y*y);
  }	
  
  float unscaledRandNum = x*( sqrt(-2*log(s) / s) );
  int constrainedValue = int( constrain( (unscaledRandNum*variance) + mean, 250, 3950) );
  return int(constrainedValue);
}