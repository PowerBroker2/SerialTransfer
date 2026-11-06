#include "SPITransfer.h"


SPITransfer myTransfer;

const int fileSize = 2000;
char file[fileSize];
char fileName[10];

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
    
    if (!myTransfer.currentPacketID())
    {
      myTransfer.rxObj(fileName);
      Serial.println();
      Serial.println(fileName);
    }
    else if (myTransfer.currentPacketID() == 1)
      for(uint8_t i=2; i<myTransfer.bytesRead; i++)
        Serial.print((char)myTransfer.packet.rxBuff[i]);
    Serial.println();
  }
}


ISR (SPI_STC_vect)
{
  if(myTransfer.available())
    procNewPacket = true;
}
