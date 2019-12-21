#include "SerialTransfer.h"


SerialTransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;

uint16_t arr[3] = {0, 1, 2};


void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);

  testStruct.z = '|';
  testStruct.y = 4.5;
}


void loop()
{
  // use this variable to keep track of how many
  // bytes we're stuffing in the transmit buffer
  uint16_t sendSize = 0;

  ///////////////////////////////////////// Stuff buffer with individual bytes
  myTransfer.txBuff[0] = 'h';
  myTransfer.txBuff[1] = 200;
  sendSize += 2;

  ///////////////////////////////////////// Stuff buffer with struct
  myTransfer.txObj(testStruct, sizeof(testStruct), sendSize);
  sendSize += sizeof(testStruct);

  ///////////////////////////////////////// Stuff buffer with array
  myTransfer.txObj(arr, sizeof(arr), sendSize);
  sendSize += sizeof(arr);

  ///////////////////////////////////////// Send buffer
  myTransfer.sendData(sendSize);
  delay(100);
}

