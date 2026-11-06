#include "SerialTransfer.h"


SerialTransfer myTransfer;


/////////////////////////////////////////////////////////////////// Callbacks
void hi()
{
  Serial.println("hi");
}

// supplied as a reference - persistent allocation required
const functionPtr callbackArr[] = { hi };
///////////////////////////////////////////////////////////////////


void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);

  ///////////////////////////////////////////////////////////////// Config Parameters
  configST myConfig;
  myConfig.debug        = true;
  myConfig.callbacks    = callbackArr;
  myConfig.callbacksLen = sizeof(callbackArr) / sizeof(functionPtr);
  /////////////////////////////////////////////////////////////////
  
  myTransfer.begin(Serial1, myConfig);
}


void loop()
{
  myTransfer.tick();
}
