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
  myTransfer.txBuff[0] = 'h';
  myTransfer.txBuff[1] = 'i';
  myTransfer.txBuff[2] = '\n';
  
  myTransfer.sendData(3);
  delay(100);
}
