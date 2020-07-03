#include "SerialTransfer.h"
#include <SoftwareSerial.h>


SoftwareSerial mySerial(2, 3); // RX, TX
SerialTransfer myTransfer;


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
    Serial.println("New Data");
    for(byte i = 0; i < myTransfer.bytesRead; i++)
      Serial.write(myTransfer.rxBuff[i]);
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
