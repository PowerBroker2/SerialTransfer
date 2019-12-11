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
  float myFloat = 100.5;
  
  myTransfer.txBuff[0] = 'h';
  myTransfer.txBuff[1] = 'i';
  myTransfer.txBuff[2] = '\n';
  myTransfer.txFloat(myFloat, 3); //insert the float "myFloat" at index 3 since "hi\n" already takes up indicies 0-2
  
  myTransfer.sendData(7); //3 bytes for "hi\n" plus 4 bytes for the float "myFloat"
  delay(100);
}
