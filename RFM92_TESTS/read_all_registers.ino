#include <SPI.h>

int _slaveSelectPin = 10; 
String content = "";
char character;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the pins
  pinMode( _slaveSelectPin, OUTPUT);
  Serial.begin(9600);
  SPI.begin();

  delay(3000);  // Wait for me to open serial monitor
}

// the loop routine runs over and over again forever:
void loop() {   
  while (Serial.available() > 0) {
     // read the incoming byte:
    character = (char)Serial.read();
    content.concat(character);
  
    if (!Serial.available()) {
      if(content == "1"){
        Serial.println("Reading all registers");
        readAllRegs();
      }
      content = "";
    } 
  }
}

/////////////////////////////////////
//    Method:   Read Register
//////////////////////////////////////

byte readRegister(byte addr)
{
  select();
  SPI.transfer(addr & 0x7F);
  byte regval = SPI.transfer(0);
  unselect();
  return regval;
}

/////////////////////////////////////
//    Method:   Select Transceiver
//////////////////////////////////////
void select() 
{
  digitalWrite(_slaveSelectPin, LOW);
}

/////////////////////////////////////
//    Method:   UNSelect Transceiver
//////////////////////////////////////
void unselect() 
{
  digitalWrite(_slaveSelectPin, HIGH);
}

/////////////////////////////////////
//    Method:   Read ALL Registers
//////////////////////////////////////
void readAllRegs( )
{
  byte regVal;
        
  for (byte regAddr = 1; regAddr <= 0x46; regAddr++)
  {
    select();
    SPI.transfer(regAddr & 0x7f);        // send address + r/w bit
    regVal = SPI.transfer(0);
    unselect();
  
    Serial.print(regAddr, HEX);
    Serial.print(" - ");
    Serial.print(regVal,HEX);
    Serial.print(" - ");
    Serial.println(regVal,BIN);
  }
}
