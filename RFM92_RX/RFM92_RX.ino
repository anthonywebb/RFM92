#include <SPI.h>

int _slaveSelectPin = 10; 
String content = "";
char character;
int dio0 = 3;
int dio5 = 2;
byte currentMode = 0x81;
char msg[11];

#define REG_FIFO                    0x00
#define REG_FIFO_ADDR_PTR           0x0D 
#define REG_FIFO_TX_BASE_AD         0x0E
#define REG_FIFO_RX_BASE_AD         0x0F
#define REG_RX_NB_BYTES             0x13
#define REG_OPMODE                  0x01
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS               0x12
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_MODEM_CONFIG            0x1D
#define REG_PAYLOAD_LENGTH          0x22
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_HOP_PERIOD              0x24

// MODES
#define RF92_MODE_RX_CONTINUOS      0x85
#define RF92_MODE_TX                0x83
#define RF92_MODE_SLEEP             0x80
#define RF92_MODE_STANDBY           0x81

#define PAYLOAD_LENGTH              0x0A
#define IMPLICIT_MODE               0x0C

// POWER AMPLIFIER CONFIG
#define REG_PA_CONFIG               0x09
#define PA_MAX_BOOST                0x8F
#define PA_LOW_BOOST                0x81
#define PA_MED_BOOST                0x8A
#define PA_OFF_BOOST                0x00

// LOW NOISE AMPLIFIER
#define REG_LNA                     0x0C
#define LNA_MAX_GAIN                0x23  // 0010 0011
#define LNA_OFF_GAIN                0x00


// the setup routine runs once when you press reset:
void setup() {                
  // initialize the pins
  pinMode( _slaveSelectPin, OUTPUT);
  pinMode(dio0, INPUT);
  pinMode(dio5, INPUT);
  Serial.begin(9600);
  SPI.begin();

  delay(3000);  // Wait for me to open serial monitor

  // LoRa mode 
  setLoRaMode();
  
  startReceiving();

  Serial.println("Setup Complete");
}

// the loop routine runs over and over again forever:
void loop() {  
  
  if(digitalRead(dio0) == 1)
  {
     receiveMessage(msg);
     Serial.print(msg);
     Serial.println("\n");
  }
   
  while (Serial.available() > 0) {
     // read the incoming byte:
    character = (char)Serial.read();
    content.concat(character);
  
    if (!Serial.available()) {
      if(content == "1"){
        Serial.println("Reading all registers");
        readAllRegs();
      }
      else if (content == "3"){
        Serial.print("DIO0 value is: ");
        Serial.println(digitalRead(dio0));
      }
      else if (content == "4"){
        Serial.print("DIO5 value is: ");
        Serial.println(digitalRead(dio5));
      }
      else if (content == "5"){
        Serial.print("Moving back to the beginning of fifo");
        writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_BASE_AD));
      }
      else if (content == "6"){
        Serial.print(readRegister(REG_FIFO));
      }
      else if (content == "7"){
        // LoRa mode 
        setLoRaMode();
        startReceiving();
      }
  
      content = "";
    } 
  }
}

/////////////////////////////////////
//    Method:   Receive FROM BUFFER
//////////////////////////////////////
void receiveMessage(char *message)
{
  
  // clear the rxDone flag
  writeRegister(REG_IRQ_FLAGS, 0x40); 
  
  int x = readRegister(REG_IRQ_FLAGS); // if any of these are set then the inbound message failed
  //Serial.println(x);
  
  // check for payload crc issues (0x20 is the bit we are looking for
  if((x & 0x20) == 0x20)
  {
    Serial.println("Oops there was a crc problem!!");
    Serial.println(x);
    // reset the crc flags
    writeRegister(REG_IRQ_FLAGS, 0x20); 
  }
  else{
    byte currentAddr = readRegister(REG_FIFO_RX_CURRENT_ADDR);
    byte receivedCount = readRegister(REG_RX_NB_BYTES);
    Serial.print("Packet! RX Current Addr:");
    Serial.println(currentAddr);
    //Serial.print("Number of bytes received:");
    //Serial.println(receivedCount);

    writeRegister(REG_FIFO_ADDR_PTR, currentAddr);   
    // now loop over the fifo getting the data
    for(int i = 0; i < receivedCount; i++)
    {
      message[i] = (char)readRegister(REG_FIFO);
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
//    Method:   Write Register
//////////////////////////////////////

void writeRegister(byte addr, byte value)
{
  select();
  SPI.transfer(addr | 0x80); // OR address with 10000000 to indicate write enable;
  SPI.transfer(value);
  unselect();
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

/////////////////////////////////////
//    Method:   Change the mode
//////////////////////////////////////
void setMode(byte newMode)
{
  if(newMode == currentMode)
    return;  
  
  switch (newMode) 
  {
    case RF92_MODE_RX_CONTINUOS:
      writeRegister(REG_PA_CONFIG, PA_OFF_BOOST);  // TURN PA OFF FOR RECIEVE??
      writeRegister(REG_LNA, LNA_MAX_GAIN);  // MAX GAIN FOR RECIEVE
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      Serial.println("Changing to Receive Continous Mode");
      break;
    case RF92_MODE_TX:
      writeRegister(REG_LNA, LNA_OFF_GAIN);  // TURN LNA OFF FOR TRANSMITT
      writeRegister(REG_PA_CONFIG, PA_MAX_BOOST);    // TURN PA TO MAX POWER 
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      Serial.println("Changing to Transmit Mode");
      break;
    case RF92_MODE_SLEEP:
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      Serial.println("Changing to Sleep Mode"); 
      break;
    case RF92_MODE_STANDBY:
      writeRegister(REG_OPMODE, newMode);
      currentMode = newMode; 
      Serial.println("Changing to Standby Mode");
      break;
    default: return;
  } 
  
  if(newMode != RF92_MODE_SLEEP){
    while(digitalRead(dio5) == 0)
    {
      Serial.print("z");
    } 
  }
  
  Serial.println(" Mode Change Done");
  return;
}

/////////////////////////////////////
//    Method:   Enable LoRa mode
//////////////////////////////////////
void setLoRaMode()
{
  Serial.println("Setting LoRa Mode");
  setMode(RF92_MODE_SLEEP);
  writeRegister(REG_OPMODE,0x80);
   
  Serial.println("LoRa Mode Set");
  return;
}

/////////////////////////////////////
//    Method:   Setup to receive continuously
//////////////////////////////////////
void startReceiving()
{
  // Turn on implicit header mode and set payload length
  writeRegister(REG_MODEM_CONFIG,IMPLICIT_MODE);
  writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);
  writeRegister(REG_HOP_PERIOD,0xFF);
  writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_BASE_AD));   
  
  // Setup Receive Continous Mode
  setMode(RF92_MODE_RX_CONTINUOS); 
}
