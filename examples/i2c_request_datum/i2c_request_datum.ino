#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct __attribute__((packed)) STRUCT {
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
  Wire.begin(); // Begin I2C as controller (master)

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
  // Request datum from connected peripheral at address 0x10
  // Callback function hi() will be called on reception
  myTransfer.requestDatum(sizeof(testStruct), 0x10);
  delay(500);
}
