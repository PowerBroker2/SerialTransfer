#include "SerialTransfer.h"
#include <SoftwareSerial.h>


SoftwareSerial mySerial(2, 3); // RX, TX
SerialTransfer myTransfer;


void setup()
{
  Serial.begin(115200);
  mySerial.begin(9600);
  myTransfer.begin(mySerial);
}


void loop()
{
  if(myTransfer.available())
  {
    Serial.println("New Data");
    for(byte i = 0; i < myTransfer.bytesRead; i++)
      Serial.write(myTransfer.rxBuff[i]);
    Serial.println();
  }
  else if(myTransfer.status < 0)
  {
    Serial.print("ERROR: ");
    Serial.println(myTransfer.status);
  }
}
