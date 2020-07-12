#include "SPITransfer.h"


SPITransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;

volatile bool procNewPacket = false;


void setup()
{
  Serial.begin(115200);
  
  SPCR |= bit (SPE);
  pinMode(MISO, OUTPUT);
  SPI.attachInterrupt();
  
  myTransfer.begin(SPI);
}


void loop()
{
  if(procNewPacket)
  {
    procNewPacket = false;
    
    myTransfer.rxObj(testStruct);
    Serial.print(testStruct.z);
    Serial.println(testStruct.y);
  }
}


ISR (SPI_STC_vect)
{
  if(myTransfer.available())
    procNewPacket = true;
}
