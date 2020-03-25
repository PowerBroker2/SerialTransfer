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
}


void loop()
{
  if(myTransfer.available())
  {
    myTransfer.rxObj(testStruct, sizeof(testStruct));
    Serial.print(testStruct.z);
    Serial.print(' ');
    Serial.println(testStruct.y);
    Serial.println();
  }
  else if(myTransfer.status < 0)
  {
    Serial.print("ERROR: ");
    Serial.println(myTransfer.status);
  }
}

