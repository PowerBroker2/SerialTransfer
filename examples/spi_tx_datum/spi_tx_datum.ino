#include "SPITransfer.h"


SPITransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;


void setup()
{
  Serial.begin(115200);
  while(!Serial);
  
  digitalWrite(SS, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);

  myTransfer.begin(SPI);

  testStruct.z = '$';
  testStruct.y = 4.5;
}


void loop()
{
  myTransfer.sendDatum(testStruct);
  delay(500);
}
