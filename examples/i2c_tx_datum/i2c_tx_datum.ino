#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;


void setup()
{
  Serial.begin(115200);
  Wire.begin();

  myTransfer.begin(Wire);

  testStruct.z = '$';
  testStruct.y = 4.5;
}


void loop()
{
  myTransfer.sendDatum(testStruct);
  delay(500);
}
