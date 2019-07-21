#include "SerialTransfer.h"

#define NUM_BYTES 3

SerialTransfer myTransfer;

uint8_t data[NUM_BYTES] = {'h', 'i', '\n'};

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);
}

void loop()
{
  myTransfer.txBuff[0] = 'h';
  myTransfer.txBuff[1] = 'i';
  myTransfer.txBuff[2] = '\n';
  
  myTransfer.sendData(NUM_BYTES);
  delay(100);
}
