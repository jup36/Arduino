//This code generates a high frequency pulse trains to trigger point grey camera image captures. 
// DIO1 receives trialOff TTL pulses that will terminate the ongoing pulse generation, which will be resumed after 3 sec (just by this time elapse). 
// The trialOff TTL pulse needs to be of 5V or smaller. By default, pulse generation will start with uploading of this script. 
// In a randomly selected trials, a 4000ms static pulse (DIO3) is generated to trigger laser. 
// The laser pulse onset is synchronized with the onset of the camera trigger pulses (PIN20). 
// Modified on 5/1/19 to include pseudoLaser pulse to make time alignments in data analysis easier. 


#define VERSION "20170417"

// VERSIONS
//  20170417 sws
// - basic template code

// ============================================
// === D E F I N E S   and  I N C L U D E S ===
// ============================================

#define USE_MAX11300   // define if MAX11300 is installed
#define USE_LCD        // define if LCD is inatalled

#include "SPI.h"    // needed for MAX11300, LCD, Auxilliary connector

//======================================
//=== P I N    D E F I N I T I O N S ===
//======================================

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
#define Valve3pin   17  // Solenoid 3 pin
#define LCDCSpin  18  // Chip Select for LCD
#define AUXCSpin  19  // Chip Select for auxilliary connector
//#define A6pin     20  // Analog in A6, IO 20
#define PIN20     20
#define DIO6      20
#define A7pin     21  // Analog in A7, IO 21
#define PIN21     21
#define DIO8      21
#define Valve1pin   22  // Solenoid 1 pin
#define Valve2pin   23  // Solenoid 2 pin
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

//======================
//=== TASK VARIABLES ===
//======================
boolean trialOff = false;

unsigned long time;   // all values that will do math with time need to be unsigned long datatype
unsigned long trialOffTime = 0; // time when the trigger pulse turned off
unsigned long laserOnTime = 0; // time when the laser pulse goes on 
int sampleFreq = 250; // video sampling frequency (default: 300 Hz)
//int pulseWidth = 1;    // width of the trigger pulse (default: 1 ms)
float dutyCycle; // duty cycle 256 equals to 100% duty cycle
int trigOffDur = 3500; // dur to keep the video capture off before it restarts the next capture
int trigOffDelay = 500;  // dur to keep the pulses on after the trial end signal from the apparatus to capture the later part of the movement
int laserTrigDur = 4000; // laser duration 
int randLaserEvery = 3; // random laser pulse generation frequency (every 5 trials, i.e., randomly 20% on average)
int pulseState = 0;

//unsigned long trigOnTime = 0; // trigger pulse train on time

//==================
//=== S E T U P  ===
//==================

void setup(void) {
  SPI.begin();     // set up SPI bus

  Serial.begin(115200);         // USB coms
  long unsigned serial_start = millis ();
  while (!Serial && ((millis () - serial_start) <= 2000));   // wait a bit for serial, but not forever if it is not connected
  Serial.println("Hello!");   // and test

  pinMode(PB1pin, INPUT_PULLUP); // go to the idle state
  pinMode(PB2pin, INPUT_PULLUP); // escape the idle state 
  //pinMode(PB3pin, INPUT_PULLUP); // the 3rd push button currently unused

  pinMode(IDSENSEpin, INPUT);   // will switch to analog in if needed

  digitalWrite(PB1pin, HIGH); // go to the idle state
  digitalWrite(PB2pin, HIGH); // escape the idle state 
  digitalWrite(AUXCSpin, HIGH); // SPI setups
  //digitalWrite(LCDCSpin, HIGH);  // LCD chip select is opposite normal SPI
  digitalWrite(MAXCSpin, HIGH);
  digitalWrite(MOSIpin, HIGH);
  digitalWrite(SCLKpin, LOW);
  digitalWrite(MAXCVTpin , HIGH); // MAX ADC convert pin
  //  pinMode(LCDCSpin, OUTPUT);
  pinMode(AUXCSpin, OUTPUT);
  pinMode(MAXCSpin, OUTPUT);
  pinMode(SCLKpin, OUTPUT);
  pinMode(MOSIpin, OUTPUT);
  pinMode(MISOpin, INPUT);
  pinMode(MAXCVTpin, OUTPUT);
  pinMode(DIOports[0], INPUT); // DIO1 trial offset input channel
  digitalWrite(DIOports[0], LOW);
  pinMode(DIOports[1], OUTPUT); // DIO2 pulse off output channel for BIAS (1 sec after the trial offset)
  digitalWrite(DIOports[1], LOW);
  pinMode(DIOports[2], OUTPUT); // DIO3 triggers the laser on/off 
  digitalWrite(DIOports[2], LOW); 
  pinMode(DIOports[3], OUTPUT); // DIO4 triggers the pseudolaser on/off 
  digitalWrite(DIOports[3], LOW); 
  
  analogWriteFrequency(PIN20, sampleFreq); // Teensy pin 20 set to 250 Hz

  // set the extra Digital I/Os to inputs
  //for( int i = 0; i < 11; i++)
  //{
  //    pinMode( DIOports[i], INPUT_PULLUP);
  //}

#ifdef USE_MAX11300
  MAX11300init();
  MAX11300regWrite(device_control, 0xC1);
#endif

  Serial.println( MAX11300regRead(device_control), HEX);

#ifdef USE_LCD
  // start & clear the display
  lcd.clearDisplay();
  lcd.setRotation(2);
  lcd.setTextSize(1);
  lcd.setTextColor(BLACK);
  lcd.setCursor(25, 2);
  lcd.print("miniBCSb");
  lcd.setCursor(10, 12);
  lcd.print("VER:");
  lcd.print(VERSION);
  lcd.refresh();
#endif
  
  dutyCycle  = (1 / (1000 / sampleFreq) * 256); 
  lastMillis = micros();
} // end of set up

int i = 0;
uint16_t inputs;

//==================
// ===  L O O P  ===
// =================
void loop() {
  time = millis(); 
    if( digitalRead(PB1pin) == LOW ){ // low is pushed
     Serial.println("PB1");
     while( digitalRead(PB1pin) == LOW) delay(50); // wait until released
     pulseState = 4; // go to the idle state 
    }   

    if( pulseState == 4 && digitalRead(PB2pin) == LOW ){ // low is pushed
     Serial.println("PB2");
     while( digitalRead(PB2pin) == LOW) delay(50); // wait until released
     pulseState = 0; // escape the idle state 
    }
  
  switch (pulseState) {
    case 0: // in this state, turn on the trigger pulse
      if (trialOff == false){ 
        if (random(1,randLaserEvery+1)>(randLaserEvery-1)){ // this will ensure random laser on for every 5 trials on average
           digitalWrite(DIO3, HIGH); // turn on the laser pulse
           laserOnTime = millis(); // mark the laser on time
        } 
        else {
           digitalWrite(DIO4, HIGH); // turn on the pseudolaser pulse
           laserOnTime = millis(); // mark the laser on time 
        }
      }      
      if ( trialOff == false ) { // trial offset TTL
        analogWrite(PIN20, 77); // use pin 20, duty cycle 77/256*100 = 30%, 51/256*100 = 20%
        Serial.println("PulseOn!");
        pulseState = 1; // go to the trialOff monitor state
      }
      break;

    case 1: // in this state, monitor the trialOff signal
      if ( digitalRead(DIO1) == true || time > laserOnTime + laserTrigDur ) {
         digitalWrite(DIO3, LOW); // turn off the laser pulse
         digitalWrite(DIO4, LOW); // turn off the pseudolaser pulse
      }
      if ( digitalRead(DIO1) == true) {
        trialOff = true;
        trialOffTime = millis();
        pulseState = 2;
      }
      break;

    case 2: // in this state, turn off the trigger pulses after passage of trigOffDelay (e.g. 1000ms)
      if ( time > trialOffTime + trigOffDelay) {
        analogWrite(PIN20, 0); // turn off the trigger pulse or digitalWrite(20, LOW);
        digitalWrite(DIO2, HIGH); // generate a pulse to stop BIAS 
        digitalWrite(DIO3, LOW);  // keep the laser off
        digitalWrite(DIO4, LOW);  // keep the pseudolaser off
        delay(50); 
        Serial.println("PulseOff!");
        trialOff = false;     
        pulseState = 3;
      }
      break;

    case 3: // in this state, monitor passage of trigger off duration
      digitalWrite(DIO2, LOW);
      if ( time > trialOffTime + trigOffDelay + trigOffDur) {
        analogWrite(PIN20, 0);   // keep the cam trigger pulse off;
        digitalWrite(DIO3, LOW); // keep the laser off
        digitalWrite(DIO4, LOW);  // keep the pseudolaser off
        Serial.println("PulseOff!");
        trialOff = false;     
        pulseState = 0;
      }
      
    case 4: // idle state; 
      analogWrite(PIN20, 0);   // just idle in this state until reactivated by push button 2;
      digitalWrite(DIO2, LOW); // keep the BIAS stop pulse off
      digitalWrite(DIO3, LOW); // keep the lasers pulse off
      digitalWrite(DIO4, LOW);  // keep the pseudolaser off
      break;
  }
}
