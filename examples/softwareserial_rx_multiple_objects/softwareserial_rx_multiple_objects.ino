#include "SerialTransfer.h"
#include <SoftwareSerial.h>


SoftwareSerial mySerial(2, 3); // RX, TX
SerialTransfer myTransfer;


struct STRUCT {
  char z;
  float y;
} testStruct;

uint16_t arr[3] = {};


void setup()
{
  Serial.begin(115200);
  mySerial.begin(9600);
  myTransfer.begin(mySerial);
}


void loop()
{
  if(myTransfer.available())
  {
    // use this variable to keep track of how many
    // bytes we've processed from the receive buffer
    uint16_t recSize = 0;

    Serial.print(myTransfer.rxBuff[0]);
    Serial.print(' ');
    Serial.print(myTransfer.rxBuff[1]);
    Serial.print(" | ");
    recSize += 2;

    myTransfer.rxObj(testStruct, sizeof(testStruct), recSize);
    Serial.print(testStruct.z);
    Serial.print(' ');
    Serial.print(testStruct.y);
    Serial.print(" | ");
    recSize += sizeof(testStruct);

    myTransfer.rxObj(arr, sizeof(arr), recSize);
    Serial.print(arr[0]);
    Serial.print(' ');
    Serial.print(arr[1]);
    Serial.print(' ');
    Serial.println(arr[2]);
    Serial.println();
  }
  else if(myTransfer.status < 0)
  {
    Serial.print("ERROR: ");
    Serial.println(myTransfer.status);
  }
}
