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

  /*
    Or, use the full constructor:
            myTransfer.begin(Serial1, true, Serial, 50);
    With the timeout parameter set to 50ms, a packet must be fully received and parsed within 50ms,
    or it will be discarded.
    The timeout value should depend on the baud rate and on the application.
    Example back-of-the-envelope calculation:
    115200bps = 14400Bps
    One packet = 264B (max) should take max 264/11400 s = 0.02s = 20ms
    to transfer. Include some time for parsing the packet (which depends on the frequency 
    of whatever task is calling transfer.available()) - and 50ms does not sound unreasonable.
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
