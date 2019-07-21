#include "SerialTransfer.h"

#define NUM_BYTES  3

SerialTransfer myTransfer;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);
}

void loop()
{
  int8_t result = myTransfer.available();
  if(result == NEW_DATA)
  {
    Serial.println("New Data");
  }
  else if((result != NO_DATA) && (result != CONTINUE))
  {
    Serial.print("ERROR: "); Serial.println(result);
  }

  /*while(Serial1.available())
    Serial.println(Serial1.read(), HEX);*/
}
