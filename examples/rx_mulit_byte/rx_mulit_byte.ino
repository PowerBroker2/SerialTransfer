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

    if(myTransfer.status == -1)
      Serial.println(F("CRC_ERROR"));
    else if(myTransfer.status == -2)
      Serial.println(F("PAYLOAD_ERROR"));
    else if(myTransfer.status == -3)
      Serial.println(F("STOP_BYTE_ERROR"));
  }
}

