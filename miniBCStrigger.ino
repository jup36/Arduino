// ==============================================
//  This is an example sketch for the miniBCS with Teensy 3.5
//  
//  2017 HHMI Janelia Steve Sawtelle
//
// ===============================================
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
#define Valve3pin   17  // Solenoid 3 pin
#define LCDCSpin  18  // Chip Select for LCD
#define AUXCSpin  19  // Chip Select for auxilliary connector
#define A6pin     20  // Analog in A6, IO 20
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


//===================
// === S E T U P  ===
// ==================


void setup(void) 
{
  SPI.begin();     // set up SPI bus
    
  Serial.begin(115200);         // USB coms
  long unsigned serial_start = millis ();
  while (!Serial && ((millis () - serial_start) <= 2000));   // wait a bit for serial, but not forever if it is not connected
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
  
  #ifdef USE_MAX11300
     MAX11300init();
     MAX11300regWrite(device_control, 0xC1);
  #endif   

  Serial.println( MAX11300regRead(device_control),HEX);
 
  #ifdef USE_LCD
 // start & clear the display
  lcd.clearDisplay();
  lcd.setRotation(2);
  lcd.setTextSize(1);
  lcd.setTextColor(BLACK);
  lcd.setCursor(25,2);
  lcd.print("miniBCSb");  
  lcd.setCursor(10,12); 
  lcd.print("VER:"); 
  lcd.print(VERSION);
  lcd.refresh();
  #endif

  lastMillis = micros();
}  

int i = 0;
static uint16_t touchIn = 0;
uint16_t inputs;

//==================
// ===  L O O P  ===
// =================

void loop()
{

// ================= TEENSY I/O ==================
// All but the lower ten I/Os on the board connect directly to the Teensy 
// These are good for I/Os that require speed or a specific pin feature (PWM, etc)

// ================ PUSHBUTTONS ======================
  if( digitalRead(PB1pin) == LOW ) // low is pushed
  {
     Serial.println("PB1");
     while( digitalRead(PB1pin) == LOW) delay(50); // wait until released
  }   
  if( digitalRead(PB2pin) == LOW ) 
  {
     Serial.println("PB2");
     while( digitalRead(PB2pin) == LOW) delay(50);
  }   
  if( digitalRead(PB3pin) == LOW ) 
  {
     Serial.println("PB3");
     while( digitalRead(PB3pin) == LOW) delay(50);
  }  

// ============== AUX I/O PORTS ====================
// for testing, they are all inputs - see if they have changed
// all have pullups, so = '1' with no connection

  static int lastDIO[11];
  int stat;

  for( i = 0; i < 11; i++ )
  {
      if( (stat = digitalRead(DIOports[i])) != lastDIO[i] )
      {
         lastDIO[i] = stat;
         Serial.print("Port ");
         Serial.print(i+1);
         Serial.print(" = ");
         Serial.println(stat);
      }
       
  }
  

// =========== VALVE DRIVERS  =========
// - need a power source plugged into Solenoid Power
//
  unsigned long nowMillis = millis();
   
  if(nowMillis - lastMillis > RELAY_DELAY)
  {
      lastMillis = nowMillis;   
 
      if (relayState == LOW)
         relayState = HIGH;
      else
         relayState = LOW;
      
     digitalWrite(Valve1pin, relayState);
     digitalWrite(Valve2pin, relayState);
     digitalWrite(Valve3pin, relayState);
  }


  // ============== MAXIM CHIP I/O ===============
  // the lower 10 I/Os on the board connect to a Maxim I/O chip
  // they must be accessed via MAX11300 commands


// ================ TOUCH INPUTS =========================

  if( (inputs =  MAX11300read(0)) != touchIn)
  {
    touchIn = inputs & 0x3f;  // bits 0-5 are touch 
    if( touchIn & DI1) Serial.print("D1 ");
    if( touchIn & DI2) Serial.print("D2 ");
    if( touchIn & DI3) Serial.print("D3 ");
    if( touchIn & DI4) Serial.print("D4 ");
    if( touchIn & DI5) Serial.print("D5 ");
    if( touchIn & DI6) Serial.print("D6 ");
    Serial.println("");
  }    

  // ================ PIEZO INPUT ===================

   static uint16_t lastPiezoIn = 0;
   uint16_t piezoIn = MAX11300readAnalog(PZ);
   if( (piezoIn > lastPiezoIn + 4) || (lastPiezoIn > piezoIn + 4))
   {
      Serial.print("Piezo: ");
      Serial.println(piezoIn);
   }   
   lastPiezoIn = piezoIn;   
  
  // ================ JOYSTICK INPUT ===================

   static uint16_t lastX = 0;
   static uint16_t lastY = 0;
   uint16_t joystickX = MAX11300readAnalog(JSX);
   uint16_t joystickY = MAX11300readAnalog(JSY);
   if( (joystickX > lastX + 1) || (lastX > joystickX + 1))
   {
      Serial.print("Joystick X: ");
      Serial.println(joystickX);
   }   
   lastX = joystickX;   
   if( (joystickY > lastY + 1) || (lastY > joystickY + 1))
   {
      Serial.print("Joystick Y: ");
      Serial.println(joystickY);
   }   
   lastY = joystickY;   
   
   delay(10);
}




