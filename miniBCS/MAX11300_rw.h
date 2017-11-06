#define MAX_DACDAT_BASE  0x60
//int gpo_data_15_to_0 = 0x0d;

void MAX11300initDelayus(int delay_us)
{
    delayMicroseconds(delay_us);
	
}



bool MAX11300regWrite(int regAddress8, int regData16)
{

   regAddress8 = regAddress8 << 1;
   digitalWrite(MAXCSpin, LOW);
   for( int i = 0; i < 8; i++)
   {
	   digitalWrite(MOSIpin, regAddress8 & 0x80);
	   digitalWrite(SCLKpin, HIGH);
	   regAddress8 = regAddress8 << 1;   
	   digitalWrite(SCLKpin, LOW);	   
   }	   
   for( int i = 0; i < 16; i++)
   {
     if( regData16 & 0x8000 )
	      digitalWrite(MOSIpin, HIGH );
      else
        digitalWrite(MOSIpin, LOW );
	   digitalWrite(SCLKpin, HIGH);
	   regData16 = regData16 << 1;   
	   digitalWrite(SCLKpin, LOW);	   
   }
   digitalWrite(MAXCSpin, HIGH);
   return true;
	
}


uint16_t  MAX11300regRead(int regAddress8)
{
uint16_t data = 0;

   regAddress8 = (regAddress8 << 1) | 1;
   digitalWrite(MAXCSpin, LOW);
   for( int i = 0; i < 8; i++)
   {
	   digitalWrite(MOSIpin, regAddress8 & 0x80);
	   digitalWrite(SCLKpin, HIGH);
	   regAddress8 = regAddress8 << 1;   
	   digitalWrite(SCLKpin, LOW);	   
   }	   
   for( int i = 0; i < 16; i++)
   {
	   digitalWrite(SCLKpin, HIGH);
	   data = data << 1;
	   data |= digitalRead(MISOpin);
	   digitalWrite(SCLKpin, LOW);		   
   }
   digitalWrite(MAXCSpin, HIGH);
   return data;
	
}

volatile static uint16_t outpins = 0;

void MAX11300write(int pin, int val)
{
   if( val )
      outpins |= 1<<pin;
   else
      outpins &= ~(1<<pin);

   MAX11300regWrite(gpo_data_15_to_0, outpins); 
}

uint16_t MAX11300read(int pin)
{

   return(  MAX11300regRead(gpi_data_15_to_0) ); 
   
}


bool MAX11300writeAnalog(uint8_t pin, uint16_t value) 
{
    return MAX11300regWrite((MAX_DACDAT_BASE + pin), value);
}    

uint16_t MAX11300readAnalog(uint8_t pin) 
{
   digitalWrite(MAXCVTpin , LOW);
   delayMicroseconds(1);
   digitalWrite(MAXCVTpin , HIGH);
   delayMicroseconds(100);  
  
    return( MAX11300regRead(adc_data_port_00 + pin));
}    

   
