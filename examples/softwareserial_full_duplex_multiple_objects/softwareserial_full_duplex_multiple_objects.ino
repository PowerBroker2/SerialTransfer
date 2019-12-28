#include "SerialTransfer.h"
#include <SoftwareSerial.h>


SoftwareSerial mySerial(2, 3); // RX, TX
SerialTransfer myTransfer;


struct STRUCT {
  char z;
  float y;
} testStruct;

uint16_t arr[3] = {0, 1, 2};


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
  // use this variable to keep track of how many
  // bytes we're stuffing in the transmit buffer
  uint16_t sendSize = 0;

  ///////////////////////////////////////// Stuff buffer with individual bytes
  myTransfer.txBuff[0] = 'h';
  myTransfer.txBuff[1] = 200;
  sendSize += 2;

  ///////////////////////////////////////// Stuff buffer with struct
  myTransfer.txObj(testStruct, sizeof(testStruct), sendSize);
  sendSize += sizeof(testStruct);

  ///////////////////////////////////////// Stuff buffer with array
  myTransfer.txObj(arr, sizeof(arr), sendSize);
  sendSize += sizeof(arr);

  ///////////////////////////////////////// Send buffer
  myTransfer.sendData(sendSize);
  delay(100);


  ///////////////////////////////////////// Parse new packet (if available())
  if(myTransfer.available())
  {
    Serial.println("New Data");
    for(byte i = 0; i < myTransfer.bytesRead; i++)
    {
      Serial.print(myTransfer.rxBuff[i]);
      Serial.print(' ');
    }
    Serial.println();
    Serial.println();
  }
  else if(myTransfer.status < 0)
  {
    Serial.print("ERROR: ");
    Serial.println(myTransfer.status);
  }
}
