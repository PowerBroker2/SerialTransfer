#include "SerialTransfer.h"

#define NUM_FIELDS 3
#define NUM_BYTES  1

SerialTransfer myTransfer;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1, NUM_FIELDS, NUM_BYTES, false);
}

void loop()
{
  int8_t result = myTransfer.available();
  if(result)
  {
    for(byte i = 0; i < NUM_FIELDS; i++)
      for(byte k = 0; k < NUM_BYTES; k++)
        Serial.write(myTransfer.inBuff[NUM_FIELDS][NUM_BYTES]);
  }
  else
  {
    Serial.print("ERROR: "); Serial.println(result);
  }
}
