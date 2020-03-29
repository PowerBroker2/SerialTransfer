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
