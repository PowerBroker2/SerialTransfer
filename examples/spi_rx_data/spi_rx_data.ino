#include "SPITransfer.h"


SPITransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;

char arr[6];

volatile bool procNewPacket = false;


void setup()
{
  Serial.begin(115200);
  
  SPCR |= bit (SPE);
  pinMode(MISO, OUTPUT);
  SPI.attachInterrupt();
  
  myTransfer.begin(SPI);
}


void loop()
{
  if(procNewPacket)
  {
    procNewPacket = false;
    
    // use this variable to keep track of how many
    // bytes we've processed from the receive buffer
    uint16_t recSize = 0;

    recSize = myTransfer.rxObj(testStruct, recSize);
    Serial.print(testStruct.z);
    Serial.print(testStruct.y);
    Serial.print(" | ");

    recSize = myTransfer.rxObj(arr, recSize);
    Serial.println(arr);
  }
}


ISR (SPI_STC_vect)
{
  if(myTransfer.available())
    procNewPacket = true;
}
