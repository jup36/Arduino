/*
fixLeverOp
Behavioral task waits for a serial command to begin trials and looks for some movement of a joystick to trigger
an output response.
*/

//// include the library code:
//#include <SPI.h>
//// pick up the SPI pins from the arduino header file
//#include <pins_arduino.h>

#define VERSION "20170417"

// VERSIONS
//  20170417 sws
// - basic template code 

// ============================================
// === D E F I N E S   and  I N C L U D E S ===
// ============================================

#define USE_MAX11300   // define if MAX11300 is installed
#define USE_LCD        // define if LCD is inatalled
#include "SPI.h"       // needed for MAX11300, LCD, Auxilliary connector

//=======================================
// === P I N    D E F I N I T I O N S ===
// ======================================

#define RX1pin    0   // Serial 1 RX 
#define TX1pin    1   // Serial 1 TX
#define ENCApin   2
#define DIO10     2   // DIO10
#define CANTXpin  3   // CAN TX or Etherenet
#define CANRXpin  4   // CAN RX or Ethernet
#define ENCBpin   5
#define DIO9      5   // DIO9
#define MAXCSpin  6   // Chip Select for MAX112300
#define RX3pin    7   // Serial 3 RX or 
#define DIO4      7   //   IO pin, with PWM
#define TX3pin    8   // Serial 3 TX or
#define DIO3      8   //   IO pin with PWM
#define RX2pin    9   // Serial 2 RX or
#define DIO2      9   //   IO pin with PWM
#define TX2pin    10  // Serial 3 TX or
#define DIO1      10  //   IO pin with PWM
#define MOSIpin   11  // SPI MOSI
#define MISOpin   12  // SPI MISO
#define SCLKpin   13  // SPI Clock
#define A0pin     14  // Analog in A0, I/O 14, PWM
#define PIN14     14
#define DIO5      14
#define A1pin     15  // Analog in A1, I/O 15
#define PIN15     15
#define DIO7      15
#define SOL4pin   16  // Solenoid 4 pin
#define Valve3pin 17  // Solenoid 3 pin
#define LCDCSpin  18  // Chip Select for LCD
#define AUXCSpin  19  // Chip Select for auxilliary connector
#define A6pin     20  // Analog in A6, IO 20
#define PIN20     20
#define DIO6      20
#define A7pin     21  // Analog in A7, IO 21
#define PIN21     21
#define DIO8      21
#define Valve2pin   22  // Solenoid 2 pin
#define Valve1pin   23  // Solenoid 1 pin
#define ETH24pin  24  // reserved for Ethernet
#define ETH25pin  25  // reserved for Ethernet
#define ETH26pin  26  // reserved for Ethernet
#define ETH27pin  27  // reserved for Ethernet
#define ETH28pin  28  // reserved for Ethernet
#define MAXINTpin 29  // interupt in from MAX11300
#define MAXCVTpin 30  // ADC Convert signal to MAX11300
#define IDSENSEpin  31 // ID sense from auxilliary connector
#define PB1pin    32  // pushbutton 1
#define PB2pin    33  // pushbutton 2
#define PB3pin    34  // pushbutton 3
#define IDXpin    35  // Encoder Index in
#define DIO11     35
#define AUX1pin   36  // Auxilliary I/O, analog, PWM for auxilliary connector
#define aux2pin   37  // Auxilliary I/O, analog, PWM for auxilliary connector
#define LED1pin   38  // User LED output
#define ETH39pin  39  // reserved for Ethernet

#define DAC0pin   A21 // DAC0 out pin (must be jumpered)
#define DAC1pin   A22 // DAC1 out pin (must be jumpered)

// MAXIM chip I/Os
// inputs are reversed on each connector, so fix it here
#define DI1   0x02   
#define DI2   0x01
#define DI3   0x08
#define DI4   0x04
#define DI5   0x20
#define DI6   0x10
#define PZ    6
#define JSX   8
#define JSY   9

// put extra ports in array so we can loop through them
static int DIOports[] = { DIO1, DIO2, DIO3, DIO4, DIO5, DIO6, DIO7, DIO8, DIO9, DIO10, DIO11 };

//=================================================================
// === I N C L U D E  P E R I P H E R A L S   I F   N E E D E D ===
// ================================================================

#ifdef USE_MAX11300
  #define MAX_SPI_MODE SPI_MODE0
  #include "MAX11300Hex.h" //YOUR_MAX_CONFIG.h"  // !!! This should be the configuration file generated by MAX11300 Configuration SW  
  #include "MAX11300_rw.h"      // do not alter this as it it has the read/write routines for the MAX11300
#endif

#ifdef USE_LCD 
  #include <Adafruit_GFX.h>
  #include <Adafruit_SharpMem.h>
  Adafruit_SharpMem lcd(SCLKpin, MOSIpin, LCDCSpin); // instantiate the LCD
  #define BLACK 0
  #define WHITE 1
#endif

unsigned long lastMillis; 
#define RELAY_DELAY  2000
int relayState = 0;

// #define BEAM1     36 // definitions in the old version
// #define SOLENOID1 22
// #define ADC_PIN   49
// #define DAC_PIN   48

//=======================
// Set the protocol name
char protocolName[] = "FixLever_v10_LL"; // should be less than 20 characters
unsigned long beginLoopTime;
unsigned long loopTime;
//=======================

//=======================
// Arrays that will hold the current values of the AO and DIO
int CurrAIValue[] = {0,0}; // JSX and JSY
//int aiThresh[] =  {512,512,512,512};
int bufferLength = 100;
int bufferEndIndex = bufferLength-1;
int frozen=0;
int first=0;
int bufferAI0[100];
int bufferAI1[100];
//int bufferAI2[100];
//int bufferAI3[100];
int bufferLoc = 0;
//=======================

//=======================
// PARAMETERS OF TASK
unsigned long interTrialInterval = 500;//15000;
unsigned long stimDelay = 0;//500;
unsigned long stimDuration = 0;//500;
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
//=======================

//=======================
// VARIABLES OF TASK
unsigned long time; // all values that will do math with time need to be unsigned long datatype
unsigned long crossTime = 0;
unsigned long rewardDelivered = 0;
boolean firstLickLogic = false;
unsigned long trialEnd = 0;
unsigned long trialStart = 0;
unsigned long firstLickTime = 0;
int ParadigmMode = 0;
int testMode = 0;
int jsZeroX = 512;
int jsZeroY = 512;
int xDisp = 0;
int yDisp=0;
int count = 0;
long displacement = 0;
//=======================

//=======================
// VARIABLES FOR SERIAL COM
int code = 0;
int assertPinNum = 0;
int assertPinState = 0;
//=======================

//===================
// === S E T U P  ===
// ==================
void setup() {
      //=======================
      //mini BCS PART
        SPI.begin();     // set up SPI bus
    
        Serial1.begin(115200);       // USB coms
        while(!Serial);             // wait for it to be ready
        Serial.println("Hello!");   // and test 

        pinMode(PB1pin, INPUT_PULLUP);  // set up pushbuttons
        pinMode(PB2pin, INPUT_PULLUP);
        pinMode(PB3pin, INPUT_PULLUP);  

        pinMode(IDSENSEpin, INPUT);   // will switch to analog in if needed
  
        digitalWrite(Valve1pin, LOW);   // Solenoids
        digitalWrite(Valve2pin, LOW);
        digitalWrite(Valve3pin, LOW);  
        digitalWrite(SOL4pin, LOW); 
        pinMode(Valve1pin, OUTPUT);  
        pinMode(Valve2pin, OUTPUT);  
        pinMode(Valve3pin, OUTPUT);
        pinMode(SOL4pin, OUTPUT);
  
        digitalWrite(AUXCSpin,HIGH);  // SPI setups
        //digitalWrite(LCDCSpin, HIGH);  // LCD chip select is opposite normal SPI
        digitalWrite(MAXCSpin, HIGH);
        digitalWrite(MOSIpin, HIGH);
        digitalWrite(SCLKpin, LOW);
        digitalWrite(MAXCVTpin , HIGH);  // MAX ADC convert pin 
        //  pinMode(LCDCSpin, OUTPUT);
        pinMode(AUXCSpin, OUTPUT);
        pinMode(MAXCSpin, OUTPUT);
        pinMode(SCLKpin, OUTPUT);
        pinMode(MOSIpin, OUTPUT);
        pinMode(MISOpin, INPUT);
        pinMode(MAXCVTpin, OUTPUT);

        // set the extra Digital I/Os to inputs 
        for( int i = 0; i < 11; i++)
        {
            pinMode( DIOports[i], INPUT_PULLUP);
        }

        //=======================
        // prep ANALOG inputs
        analogReference(DEFAULT);
        //=======================

        //=======================
        // initialize the SERIAL communication
        Serial.begin(115200);
        //=======================

        //=======================
        // initialize SERIAL for LCD
        Serial1.begin(115200); 
        Serial1.println(protocolName); // print protocol name
        //=======================
        
        #ifdef USE_MAX11300
          MAX11300init();
          MAX11300regWrite(device_control, 0xC1);
        #endif   

        Serial1.println( MAX11300regRead(device_control),HEX);

        #ifdef USE_LCD
        // start & clear the display
        lcd.clearDisplay();
        lcd.setRotation(2);
        lcd.setTextSize(1);
        lcd.setTextColor(BLACK);
        lcd.setCursor(25,2);
        lcd.print("miniBCS");  
        lcd.setCursor(10,12); 
        lcd.print("VER:"); 
        lcd.print(VERSION);
        lcd.refresh();
        #endif

        //lastMillis = micros();

        //=======================
        // initialize zero positions
        jsZeroX = MAX11300readAnalog(JSX);
        jsZeroY = MAX11300readAnalog(JSY);
        //=======================

        count = 0; // initialize the trial counter
        
        //=======================
        // initialize analog read buffer
        for (int j=0;j<bufferLength;j++) {
          bufferAI0[j] = 0;
          bufferAI1[j] = 0;    
        }
        bufferLoc = 0;
        //=======================
      
        CurrAIValue[0] = MAX11300readAnalog(JSX);
        CurrAIValue[1] = MAX11300readAnalog(JSY);
}  

//==========================================================================================================================
// MAIN EXECUTION LOOP
//==========================================================================================================================
void loop() {
  
      beginLoopTime = micros();
      time = millis();         

      if(frozen==0) {
          // Left shift the buffer
          for (int i=1;i<bufferLength;i++) {
            bufferAI0[i-1] = bufferAI0[i];
            bufferAI1[i-1] = bufferAI1[i];
          }
          
          bufferAI0[bufferEndIndex] = int(MAX11300readAnalog(JSX)); // joystick x ... takes 110us
          bufferAI1[bufferEndIndex] = int(MAX11300readAnalog(JSY)); // joystick y
      
          CurrAIValue[0] = bufferAI0[bufferEndIndex]; 
          CurrAIValue[1] = bufferAI1[bufferEndIndex];
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
            jsZeroX = CurrAIValue[0];
            jsZeroY = CurrAIValue[1];
            trialStart = time;
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
              case 0: // default case 
                xDisp = abs(CurrAIValue[0]-jsZeroX);
                yDisp = abs(CurrAIValue[1]-jsZeroY);
                if (xDisp>xWidth || yDisp>yWidth) {
                  
                  frozen = 0;        
                  crossTime = time;
                  firstLickLogic = true;
                  ParadigmMode = 3;
                  Serial1.write(byte(0x0c));   // clear the display
                  Serial1.write(byte(0x94));  
                  Serial1.print("EVENT");
                }
                break;
              
              //case 1: // this is the "windowed" reach case (deprecated), reach magnitude has to be put within a range (window)
                //xDisp = (CurrAIValue[0]-jsZeroX);
                //yDisp = (CurrAIValue[1]-jsZeroY);
                //if (xDisp > (xCenter-xWidth) && xDisp < (xCenter+xWidth)) {
                  //if (yDisp > (yCenter-yWidth) && yDisp < (yCenter+yWidth)) {
                    //valveOpenTime =valveOpenTimeX;
                    //frozen = 0;        
                    //crossTime = time;
                    //firstLickLogic = true;
                    //digitalWrite(DIOports[1], HIGH);  // success event pulse
                    //analogWrite(DIOports[2], 255);
                    //digitalWrite(DIOports[0], LOW); // trial cue 
                    //digitalWrite(DIOports[6], HIGH); // performance cue indicator to send to the recording system
                    //ParadigmMode = 3;
                  //}
                //}
                //break;
              
              //case 2: // deprecated
                //frozen = 0;        
                //crossTime = time;
                //firstLickLogic = true;
                //digitalWrite(DIOports[1], HIGH); // success event pulse
                //analogWrite(DIOports[2], 255);
                //digitalWrite(DIOports[0], LOW);  // trial cue 
                //digitalWrite(DIOports[6], HIGH); // performance cue indicator to send to the recording system
                //ParadigmMode = 3;
                //break;
  
              //case 3: // deprecated
                //if (xDisp>xWidth)  {
                  //valveOpenTime = valveOpenTimeX;
                  //frozen = 0;        
                  //crossTime = time;
                  //firstLickLogic = true;
                  //leftTrial = 1;
                  //digitalWrite(DIOports[1], HIGH);  // success event pulse
                  //analogWrite(DIOports[2], 255);
                  //digitalWrite(DIOports[0], LOW); // trial cue 
                  //digitalWrite(DIOports[6], HIGH); // performance cue indicator to send to the recording system
                  //ParadigmMode = 3;
                //}
                //if (xDisp<-xWidth)  {
                  //valveOpenTime = valveOpenTimeY;
                  //frozen = 0;        
                  //crossTime = time;
                  //firstLickLogic = true;
                  //leftTrial = 0;
                  //digitalWrite(DIOports[1], HIGH);  // success event pulse
                  //analogWrite(DIOports[2], 255);
                  //digitalWrite(DIOports[0], LOW); // trial cue 
                  //digitalWrite(DIOports[6], HIGH); // performance cue indicator to send to the recording system
                  //ParadigmMode = 3;
                //}
              //break;
            }
            
            break;
//==========================================================================================================================
// RULES FOR EVENT TRIGGERS
//==========================================================================================================================
    
        case 3:
            
            //if (time > (crossTime+250) ) { // deprecated
            //   analogWrite(DIOports[2], 0);              
            //}
            
            //if(CurrAIValue[3]<lThresh && first==0) { // this is to call the trialEnd as the time of the first lick
            //  trialEnd = time;
            //  first = 1;
            //}
            
            if(time > (crossTime+valveDelayTime) ) {
              digitalWrite(Valve1pin, HIGH);  // valve opens
              rewardDelivered = time;
              digitalWrite(DIOports[7], HIGH); // trial cue indicator to send to the recording system
              ParadigmMode = 4;              
              Serial1.write(byte(0x0c));   // clear the display
              Serial1.write(byte(0x94));  
              Serial1.print("REWARD");
            }

            break;
    
        case 4: // close valve once the reward has had sufficient time to be delivered
            
            //if(CurrAIValue[3]<lThresh && first==0) { // this is to call the trialEnd as the time of the first lick
            //  trialEnd = time;
            //  first = 1;
            //}
            
            if(valveOpenTime<10) {
              valveOpenTime=10;
            }
            
            if(time > (crossTime+valveDelayTime+valveOpenTime) ) {
              digitalWrite(Valve1pin, LOW);  // valve closes
              ParadigmMode = 5;              
              Serial1.write(byte(0x0c));   // clear the display
              Serial1.write(byte(0x94));  
              Serial1.print("POST-REWARD");
            }      

            break;
              
        case 5: // this is the end of trial state

            //if(CurrAIValue[3]<lThresh && first==0) { // this is to call the trialEnd as the time of the first lick
            //  trialEnd = time;
            //  first = 1;
            //}
            
            if ( time-rewardDelivered>3000 ) { // okay this (3000) might determine the iti in the task
              
              //if (first==0) {
              //  trialEnd = 3000+crossTime;
              //}

              // send a notice of trial end
              Serial.print(5);
              Serial.print(",");
              Serial.print("*");
              
              // defrost the recording buffer
              frozen = 0;
              //first = 0;

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
           
            //digitalWrite(DIOports[0], LOW); // success event
            //digitalWrite(DIOports[1], LOW); // trial cue light
            //digitalWrite(DIOports[2], LOW); // performance cue light
            //digitalWrite(DIOports[3], LOW); // valve
            //digitalWrite(DIOports[4], LOW); // reset the lick indicator
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
//    digitalWrite(DIOports[0], LOW); // trial cue light
//    digitalWrite(DIOports[2], LOW); // performance cue light
//    digitalWrite(DIOports[3], LOW); // valve 
//    digitalWrite(DIOports[1], LOW); // success event
//    digitalWrite(DIOports[4], LOW); // reset the lick indicator
//    digitalWrite(DIOports[5], LOW); // 
//    digitalWrite(DIOports[6], LOW); // 
//    digitalWrite(DIOports[7], LOW); // 
}


