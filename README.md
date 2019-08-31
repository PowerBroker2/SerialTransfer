# SerialTransfer
Arduino library to transfer packetized data fast/reliably via UART/Serial

**How To Use:**

*Both Arduinos:*
1.) Include the library and create a Serial Transfer class instance:
```c++
#include "SerialTransfer.h"

SerialTransfer myTransfer;
```

2.) Determine which serial ports the transfer will occur on and initialize it:
```c++
Serial1.begin(115200);
```

3.) Pass the SerialX class to the SerialTransfer class for initialization:
```c++
myTransfer.begin(Serial1);
```


*Transmitting Arduino:*
1.) Insert data bytes into the SerialTransfer TX buffer:
```c++
myTransfer.txBuff[0] = 'h';
myTransfer.txBuff[1] = 'i';
myTransfer.txBuff[2] = '\n';
```

2.) Transmit the data via the "sendData" member function. The argument of sendData() is the number of bytes of the TX buffer to be transmitted. Since we stuffed 3 data bytes ('h', 'i', and '\n') into the TX buffer, we need to specify 3 bytes to be transferred:
```c++
myTransfer.sendData(3);
```


*Receiving Arduino:*
1.) Repetitively check to see if a new packet has been completely received. Also, check to see if any transfer errors have occurred:
```c++
int8_t result = myTransfer.available();
if(result == NEW_DATA)
{
  // see next step
}
else if((result != NO_DATA) && (result != CONTINUE))
{
  Serial.print("ERROR: ");
  Serial.println(result);
}
```

2.) If a full packet has been received, use the SerialTransfer class's RX buffer to access the received data bytes:
```c++
Serial.println("New Data");
for(byte i = 0; i < 3; i++)
  Serial.write(myTransfer.rxBuff[i]);
Serial.println();
```
