#include "SerialTransfer.h"


SerialTransfer myTransfer;

const int fileSize = 2000;
char file[fileSize];
char fileName[10];


void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  
  myTransfer.begin(Serial1);
}


void loop()
{
  if (myTransfer.available())
  {
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