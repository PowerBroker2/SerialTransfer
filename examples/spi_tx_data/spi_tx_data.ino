#include "SPITransfer.h"


SPITransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;

char arr[] = "hello";


void setup()
{
  Serial.begin(115200);
  
  digitalWrite(SS, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);

  myTransfer.begin(SPI);

  testStruct.z = '$';
  testStruct.y = 4.5;
}


void loop()
{
  // use this variable to keep track of how many
  // bytes we're stuffing in the transmit buffer
  uint16_t sendSize = 0;

  ///////////////////////////////////////// Stuff buffer with struct
  sendSize = myTransfer.txObj(testStruct, sendSize);

  ///////////////////////////////////////// Stuff buffer with array
  sendSize = myTransfer.txObj(arr, sendSize);

  ///////////////////////////////////////// Send buffer
  myTransfer.sendData(sendSize);
  delay(500);
}
