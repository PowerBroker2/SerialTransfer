#include "SerialTransfer.h"

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
    for(byte i = 0; i < 3; i++)
      Serial.print(myTransfer.rxBuff[i]);
    Serial.println();
  }
  else if((result != NO_DATA) && (result != CONTINUE))
  {
    Serial.print("ERROR: ");
    Serial.println(result);
  }
}
