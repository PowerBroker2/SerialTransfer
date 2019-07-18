#include "SerialTransfer.h"

#define NUM_FIELDS 3

SerialTransfer myTransfer;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1, NUM_FIELDS, 1, false);
}

void loop()
{
  uint8_t data[NUM_FIELDS] = { 'h', 'i', '\n' };
  Serial.println(myTransfer.sendData(data, (sizeof(data) / sizeof(data[0]))));
  delay(1000);
}
