#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;

char arr[6];


/////////////////////////////////////////////////////////////////// Callbacks
void hi()
{
  // use this variable to keep track of how many
  // bytes we've processed from the receive buffer
  uint16_t recSize = 0;

  recSize = myTransfer.rxObj(testStruct, recSize);
  Serial.print(testStruct.z);
  Serial.print(testStruct.y);
  Serial.print(" | ");

  recSize = myTransfer.rxObj(arr, recSize);
  Serial.println(arr);
}

// supplied as a reference - persistent allocation required
const functionPtr callbackArr[] = { hi };
///////////////////////////////////////////////////////////////////


void setup()
{
  Serial.begin(115200);
  Wire.begin(0);

  ///////////////////////////////////////////////////////////////// Config Parameters
  configST myConfig;
  myConfig.debug        = true;
  myConfig.callbacks    = callbackArr;
  myConfig.callbacksLen = sizeof(callbackArr) / sizeof(functionPtr);
  /////////////////////////////////////////////////////////////////
  
  myTransfer.begin(Wire, myConfig);
}


void loop()
{
  // Do nothing
}
