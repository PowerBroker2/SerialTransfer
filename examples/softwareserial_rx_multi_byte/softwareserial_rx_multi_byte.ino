#include "SerialTransfer.h"
#include <SoftwareSerial.h>


SoftwareSerial mySerial(2, 3); // RX, TX
SerialTransfer myTransfer;


struct STRUCT {
  char z;
  float y;
} testStruct;


void setup()
{
  Serial.begin(115200);
  mySerial.begin(9600);
  myTransfer.begin(mySerial);

  testStruct.z = '|';
  testStruct.y = 4.5;
}


void loop()
{
  if(myTransfer.available())
  {
    myTransfer.rxObj(testStruct);
    Serial.print(testStruct.z);
    Serial.print(' ');
    Serial.println(testStruct.y);
    Serial.println();
  }
  else if(!myTransfer.status)
  {
    Serial.print("ERROR: ");

    if(myTransfer.status == CRC_ERROR)
      Serial.println(F("CRC_ERROR"));
    else if(myTransfer.status == PAYLOAD_ERROR)
      Serial.println(F("PAYLOAD_ERROR"));
    else if(myTransfer.status == STOP_BYTE_ERROR)
      Serial.println(F("STOP_BYTE_ERROR"));
  }
}
