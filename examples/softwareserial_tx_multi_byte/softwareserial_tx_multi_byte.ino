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
  myTransfer.txObj(testStruct, sizeof(testStruct));
  myTransfer.sendData(sizeof(testStruct));
  delay(100);
}
