#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct __attribute__((packed)) STRUCT {
  char z;
  float y;
} testStruct;

void I2C_RequestHandler(void)
{
  myTransfer.replyWithDatum(testStruct);
}

void setup()
{
  Serial.begin(115200);

  Wire.begin(0x10); // Begin I2C as peripheral (slave) with address 0x10
  Wire.onRequest(I2C_RequestHandler); //Setup I2C_RequestHandler function as I2C request callback

  myTransfer.begin(Wire);

  testStruct.z = '$';
  testStruct.y = 4.5;
}


void loop()
{
  // Do nothing
}
