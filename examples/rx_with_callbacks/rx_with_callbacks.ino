#include "SerialTransfer.h"


SerialTransfer myTransfer;


/////////////////////////////////////////////////////////////////// Callbacks
void hi()
{
   Serial.println("hi");
}
///////////////////////////////////////////////////////////////////


void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);

  functionPtr callbackArr[] = { hi };

  ///////////////////////////////////////////////////////////////// Config Parameters
  configST myConfig;
  myConfig.port         = &Serial1;
  myConfig.debug        = true;
  myConfig.callbacks    = callbackArr;
  myConfig.callbacksLen = sizeof(callbackArr) / sizeof(functionPtr);
  /////////////////////////////////////////////////////////////////
  
  myTransfer.begin(myConfig);
}


void loop()
{
  myTransfer.tick();
}

