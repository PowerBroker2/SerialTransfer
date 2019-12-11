#include "SerialTransfer.h"

SerialTransfer myTransfer;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);
}

void loop()
{
  float myFloat;
  
  if(myTransfer.available())
  {
    /////////////////////////////////////////////////////////////// Handle Entire Packet
    Serial.println("New Data");
    for(byte i = 0; i < myTransfer.bytesRead; i++)
      Serial.write(myTransfer.rxBuff[i]);
    Serial.println();

    /////////////////////////////////////////////////////////////// Parse Out Float From Packet
    myTransfer.rxFloat(myFloat, 3);
    Serial.print("Received float: "); Serial.println(myFloat);
    Serial.println();
  }
  else if(myTransfer.status < 0)
  {
    Serial.print("ERROR: ");
    Serial.println(myTransfer.status);
  }
}
