#include "SerialTransfer.h"


SerialTransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;


void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);

  testStruct.z = '$';
  testStruct.y = 4.5;
}


void loop()
{
  myTransfer.sendDatum(testStruct);
  delay(500);
}
