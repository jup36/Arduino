
import controlP5.*;
import processing.serial.*;
import processing.video.*;
import processing.net.*; 

/*
BEHAVIOR MONITORING SOFTWARE VERSION 3
note: unlike the previous implementations, in this case the behavior monitor on the host computer is the controller 
of task execution and trial initiation. This will allow the user to have more control over current behavior execution
than previously and make almost all modifications of the paradigm on the local machine rather than changing the underlying
arduino code.

This paradigm:
Modified on Dec/17 to incorporate generation of digital pulses for laser stimulation on randomly selected trials at user defined percentage. 

REMAINING ISSUES:

*/

PrintWriter output;
PrintWriter parameters;
PrintWriter outputX;
PrintWriter outputY;
PrintWriter outputT;
PrintWriter compTraj;

int targetOn = 0;  //////    turn to 0 for normal threshold.   turn to 1 for rightleft target

long lastEvent = 0; // time variables need a sufficient sized datatype
int varA = 0;
int varB = 0;
int varC = 0;
int varD = 0;
int digA = 0;
int digB = 0;
int digC = 0;
int digD = 0;
int digE = 0;
int digF = 0;
int digG = 0;
int digH = 0;
int digI = 0;
int digJ = 0;
int digK = 0;
int digL = 0;
int digM = 0;
int buffPos=0;
int[] monD;
int pinNumber = 0;
int pinLogic = 0;
int param1 = 1000;
int param2 = 555;
int param3 = 424;
int param4 = 512;
int param5 = 255;
int dispA = 512;
int dispB = 512;
int count = 1;
int Offsets = 5;
int ButtonWidth = 52;
int isRunning = 0;
int testMode = 0;
int controlCode=100;
int betweenTrials=0;
long[] trialData = {0,0,0,0,0,0,0};
int[] trialStarts;
int[] trialEvents;
int[] trialEnds;
int trajLength = 10;
long varT = 0;
long time = 0;
long lastTrialCompletion = 0;
int gain = 2;

int[] trajX;
int[] trajY;
int[] lTrajX;
int[] lTrajY;
int[] trialD;
int[] paramWave;
boolean initialRun = true;
boolean serialEventLogic = false;
boolean leftTrial = true;
boolean endSessionLogic = false;
boolean isRecording = false;

String fileSuffix = "";
String mS=""; 
String dS="";
String data;

int interTrialInterval = 1000;//1500; (interTrialInterval is determined within 'FixLever_v10_LL_stim (see case 5)')
int stimDelay = 0;//500
int stimDuration = 0;//500;
int stimSession = 1;  // global (session) laser stim logic; 0: do NOT use laser, 1: USE laser 
int stimTrial   = 0;  // trial-by-trial laser stim logic; 0: stim OFF, 1: stim ON 
int stimPerc  = 20;   // percent of trials to be stimulated (laser ON); e.g. 20 %
int xCenter=  0;
int xWidth = 20;
int yCenter=  0;
int yWidth = 20;
int boundaryWidth = 20;
int valveOpenTime = 35;
int valveOpenTimeX = 35; //30
int valveOpenTimeY = 35; //30
int valveDelayTime = 1000;
int rightleft = int (random(1,3)); 

//int [] thresholds = { 18,22,28,32};  
//int [] thresholds = {10,16,13,20};
//int [] thresholds = {30,30,30,30};
//int [] thresholds = {31,40,46,40};
int [] thresholds = {30,40,40,30};
int blockType;
boolean threshTask = false;

int leftStreak = 1;
int rightStreak = 1;
int responseMode=0; 
int switchingTask = 0; // 1 for the switching task
int switchTrial =int (random(50,90));
//int switchTrial = 1;
int variable = 0;
int omit = 0;
int omittrial = 0;
float randomdraw;

//===========================================  
// PREPARE THE SERIAL PORT
Serial port;
String portname = Serial.list()[0];
int baudrate = 115200;
//===========================================  

//===========================================  
// BROADCAST OUT TIME STAMPS TO SYNC OTHER CLIENTS
Server myServer;
String transmittedPacket;
//===========================================  

//===========================================  
// PREPARE THE TEXT LABELS AND INPUT
ControlP5 controlP5;
Button updateB;
Button testB;
Button runB;
Button stopB;
Button recordB;
Textarea digLabel1;
Textarea digLabel2;
Textarea digLabel3;
Textarea digLabel4;
Textarea protLabel;
Textarea protName;
Textfield fileLabelS;
Textfield fileLabelP;
Numberbox TrialCounter;
Numberbox TrialStart;
Numberbox TrialEvent;
Numberbox TrialEnd;
Numberbox ParamDisp0;
Numberbox ParamDisp1;
Numberbox ParamDisp2;
Numberbox ParamDisp3;
Numberbox ParamDisp4;
Numberbox ParamDisp5;
Numberbox ParamDisp6;
Numberbox ParamDisp7;
Numberbox ParamDisp8;
Numberbox ParamDisp9;
Numberbox ParamDisp10;
//===========================================

//===========================================  
// PREPARE THE VIDEO CAPTURE SETUP
int recording=0;
int justStopped=0;
//===========================================  

//===========================================
//===========================================
// BEGIN SETUP FUNCTION
//===========================================
//===========================================
void setup() {

  //===========================================
  // DISPLAY PARAMS  
  frameRate(30);
  background(0);
  size(1120, 480);
  smooth();
  rectMode(CENTER);
  //===========================================

  //===========================================
  // INITIALIZE SERIAL
  port = new Serial(this, portname, baudrate);
  paramWave = new int[10];
  println("using serial data from port: "+portname);
  //===========================================
  
  //===========================================
  // INITIALIZE SERVER FOR TIMESTAMP BROADCAST
  myServer = new Server(this, 5204); 
  //===========================================

  //===========================================
  // INITIALIZE TRAJECTORY BUFFER AND LICK BUFFER
  lTrajX = new int[trajLength];
  lTrajY = new int[trajLength];
  monD   = new int[130];
  trialD = new int[130];
  //===========================================

  //===========================================
  // PREPARE DATA STORAGE  
  int d = day();    // Values from 1 - 31
  if (d<10) {
    dS = "0"+str(d);
  } else {
    dS = str(d);
  }
  int m = month();  // Values from 1 - 12
  if (m<10) {
    mS = "0"+str(m);
  } else {
    mS = str(m);
  }
  int y = year();   // 2003, 2004, 2005, etc.
  int h = hour();
  int mi = minute();
  fileSuffix = "_"+String.valueOf(y)+"_"+mS+"_"+dS+"_"+String.valueOf(h)+String.valueOf(mi);
  //===========================================

  //========================================  
  // SET UP THE CONTROLP5 OBJECT
  controlP5 = new ControlP5(this);
  // BUTTONS FOR EXECUTION CONTROL
  controlP5.addButton("run",1,Offsets,Offsets,ButtonWidth,14).setId(1);
  controlP5.addButton("stop",2,(Offsets+ButtonWidth)+Offsets,Offsets,ButtonWidth,14).setId(2);
  controlP5.addButton("record",3,2*(Offsets+ButtonWidth)+Offsets,Offsets,ButtonWidth,14).setId(3);
  // VIDEO RECORD
  controlP5.addButton("test",5,3*(Offsets+ButtonWidth)+Offsets,Offsets,ButtonWidth,14).setId(5);
  // LABELS FOR THE DIGITAL DISPLAY
  digLabel1 = controlP5.addTextarea("dig1","TRIAL CUE",22,220,80,10);
  digLabel2 = controlP5.addTextarea("dig2","TARGET",22,235,80,10);
  digLabel3 = controlP5.addTextarea("dig3","LICK LOGIC",22,250,80,10);
  digLabel4 = controlP5.addTextarea("dig4","REWARD",22,265,80,10);
  // NUMBERBOXES FOR VALUE DISPLAY
  TrialCounter = controlP5.addNumberbox("Trials",0,Offsets,30,ButtonWidth,14);
  TrialStart = controlP5.addNumberbox("Start",0,(Offsets+ButtonWidth)+Offsets,30,ButtonWidth,14);
  TrialEvent = controlP5.addNumberbox("Event",0,2*(Offsets+ButtonWidth)+Offsets,30,ButtonWidth,14);
  TrialEnd = controlP5.addNumberbox("End",0,3*(Offsets+ButtonWidth)+Offsets,30,ButtonWidth,14);
  // NUMBERBOXES FOR PARAMETER DISPLAY
  ParamDisp0 = controlP5.addNumberbox("P0>ITI",400,290-ButtonWidth-5,5,ButtonWidth,14);  //  this is passed
  ParamDisp1 = controlP5.addNumberbox("P1>stimDelay",500,290-ButtonWidth-5,35,ButtonWidth,14);
  ParamDisp2 = controlP5.addNumberbox("P2>stimDur",500,290-ButtonWidth-5,65,ButtonWidth,14);
  ParamDisp3 = controlP5.addNumberbox("P3>xCnt",0,290-ButtonWidth-5,95,ButtonWidth,14);
  ParamDisp4 = controlP5.addNumberbox("P4>xWide",20,290-ButtonWidth-5,125,ButtonWidth,14);  //was 13
  ParamDisp5 = controlP5.addNumberbox("P5>yCnt",0,290-ButtonWidth-5,155,ButtonWidth,14);
  ParamDisp6 = controlP5.addNumberbox("P6>yWide",20,290-ButtonWidth-5,185,ButtonWidth,14);
  ParamDisp7 = controlP5.addNumberbox("P7>ValOpen",12,290-ButtonWidth-5,215,ButtonWidth,14);
  ParamDisp8 = controlP5.addNumberbox("P8>ValDel",400,290-ButtonWidth-5,245,ButtonWidth,14);  
  ParamDisp9 = controlP5.addNumberbox("P9>RespMode",responseMode,290-ButtonWidth-5,275,ButtonWidth,14);
  ParamDisp10 = controlP5.addNumberbox("P10>rightleft",rightleft,290-ButtonWidth-5,305,ButtonWidth,14);
  
  //ParamDisp10 = controlP5.addNumberbox("P10>GAIN",gain,290-ButtonWidth-5,305,ButtonWidth,14); 
  
  // DISPLAY PROTOCOL NAME SOMEWHERE HERE /////////////////
  protLabel = controlP5.addTextarea("protLabel","PROTOCOL",5,90,80,10);
  protName = controlP5.addTextarea("protName","DEFAULT",5,105,80,10);  

  // FILE DETAILS
  fileLabelP = controlP5.addTextfield("MOUSE",5,130,65,20);
  fileLabelP.setText("0");
  fileLabelS = controlP5.addTextfield("SUFFIX",5,170,65,20);
  fileLabelS.setText(fileSuffix+" ");
  //========================================  
  
  // prepare the array of random values (range: [0 1]) to determine stim trials
  //float[] randoms = new float[100]; 
  //for (int i=0; i<100; i++) {
  //    randoms[i] = random(100);
  //}

  //========================================  
  // Run once to initialize a bunch of variables in the display and get the current protocol name
  eot_parameter();
}
// END SETUP

//======================================================================================
//======================================================================================
void draw() {
//======================================================================================
//======================================================================================
  //gain - int(ParamDisp10.value());
  time = millis();
  
  //===========================================
  // ERASE DISPLAY FOR REDRAW
  background(0);
  //===========================================

  //===========================================
  // PARSE INCOMING SERIAL DATA
  if(port.available() > 0) {
    delay(5);
    parseSerialData();
  }
  //=========================================== 


  //===========================================
  // THRESHOLD REPRESENTATION
  if( targetOn ==0){
  noFill();
  stroke(0, 255, 0);
  strokeWeight(10);
  rect(720, 240, ParamDisp4.value()*4*gain, ParamDisp6.value()*4*gain);
  strokeWeight(1);
  }
  
  if (targetOn==1){
    if(rightleft==1){ //left
        noFill();
  stroke(0, 200, 100);
  strokeWeight(40);
  //rect(820+ParamDisp4.value()*4*gain, 240,150,200);
  //rect(820+40*gain, 240, 150,100);
  triangle(820,180,900,180, 860,260);
  strokeWeight(1);
    }
    if(rightleft==2){ //right
  stroke(0, 50, 200);
  strokeWeight(70);
  //rect(620-ParamDisp4.value()*4 *gain, 240, 150,200);
  rect(620-40 *gain, 240, 100,150);
  strokeWeight(1);
    }
  }
  //===========================================
  
  //===========================================
  // JOYSTICK TRACKING
  // DISPLAY AND STORE TRAJECTORIES (SET BY trajLength)
//  strok eight(8);
  for (int i=1; i<trajLength; i++) {
    stroke(255, 255, 255, 100+(i*(100/trajLength)));
    fill(255,255,255,100+(i*(100/trajLength)));
    line(lTrajX[i-1], lTrajY[i-1],lTrajX[i], lTrajY[i]);
    ellipse(lTrajX[i], lTrajY[i], trajLength+30, trajLength+30);
    lTrajX[i-1] = lTrajX[i];
    lTrajY[i-1] = lTrajY[i]; 
  }

  lTrajX[trajLength-1] = 720+(varA*gain);
  lTrajY[trajLength-1] = 240+(varB*gain);  
  // display the current point
  noStroke();
  fill(255);
  if(betweenTrials==0 & isRunning==1) {
    ellipse(lTrajX[trajLength-1], lTrajY[trajLength-1], trajLength*3, trajLength*3);
  } 
  if(isRunning==0) {
    ellipse(lTrajX[trajLength-1], lTrajY[trajLength-1], trajLength*3, trajLength*3);
  }
  //===========================================
if(digA == 0) {
  ellipse(680,240,trajLength*75, trajLength*75); 
}
  //===========================================
  // DIGITAL MONITORS...
  stroke(255);
  strokeWeight(1);
  fill(digA*255,0,0);
  ellipse(10,220,10,10);
  fill(digC*255,0,0);
  ellipse(10,235,10,10);
  fill(digE*255,0,0);
  ellipse(10,250,10,10);
  fill(digD*255,0,0);
  ellipse(10,265,10,10);
  //===========================================

  //===========================================
  // LICK MONITOR
    monD[buffPos] = varC;
    if (buffPos==129) {
      buffPos=0;
    } else {
      buffPos++;
    
    stroke(0xff08a2cf);
    for(int i=1; i<130; i++) {
      strokeWeight(1);
      line(80+i-1,460-monD[i-1],80+i,460-monD[i]);
      strokeWeight(3);
      point(80+i-1,310-trialD[i-1]);
    }
  }
  strokeWeight(1);
  //===========================================
  
  //===========================================
  // CONTROL OF TASK EXECUTION
  switch(controlCode) {
      case 0: // startOfTrial
        sendSerialCode(2);
        controlCode=2;
        break;          
      case 1: // endOfTrial event received, go back to streaming data
        betweenTrials = 1;
        eot_parser();
        eot_parameter();
        eot_analysis();
        break;
      case 2: // request stream data
        sendSerialCode(1);
        break;
      case 3: // endOfTrial data received, now request event
        controlCode=-1;
        sendSerialCode(4);        
        break;
      case 4: // endOfTrial data received, now request event
        String stats = "SESSION HAS FINISHED \nSESSION HAS FINISHED \nSESSION HAS FINISHED \nSESSION HAS FINISHED \nSESSION HAS FINISHED \nSESSION HAS FINISHED \nSESSION HAS FINISHED \nSESSION HAS FINISHED";
        fill(255, 0, 0);
        text(stats, 10, 10);       
        break;
    }
  //===========================================  

  //===========================================  
  if(isRecording) {
    data = time + "," + varA + "," + varB + "," + varC;
    compTraj.println(data);
    compTraj.flush();
  }
  //===========================================  

  //===========================================  
  if(isRunning==1 & betweenTrials==1) {
    if(time > (lastTrialCompletion+interTrialInterval)) {
      controlCode=0;
      betweenTrials=0; 
    }
  }
  //===========================================  
  
} // END DRAW