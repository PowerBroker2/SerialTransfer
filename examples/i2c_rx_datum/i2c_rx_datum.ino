#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;


/////////////////////////////////////////////////////////////////// Callbacks
void hi()
{
  myTransfer.rxObj(testStruct);
  Serial.print(testStruct.z);
  Serial.println(testStruct.y);
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
