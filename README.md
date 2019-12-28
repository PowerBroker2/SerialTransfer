# SerialTransfer
Arduino library (available through the Arduino IDE's Libraries Manager!) to transfer packetized data fast/reliably via UART/Serial. [A mirror library is also available for Python!](https://github.com/PowerBroker2/pySerialTransfer)

This library:
- works with "software-serial" libraries
- is non blocking
- uses packet delimiters
- uses consistent overhead byte stuffing
- uses CRC-8 (Polynomial 0x9B with lookup table)
- allows the use of dynamically sized packets (packets can have payload lengths anywhere from 1 to 255 bytes)
- **can transfer bytes, ints, floats, and even structs!!**

# Packet Anatomy:
```
01111110 11111111 00000000 00000000 00000000 ... 00000000 10000001
|      | |      | |      | |      | |      | | | |      | |______|__Stop byte
|      | |      | |      | |      | |      | | | |______|___________8-bit CRC
|      | |      | |      | |      | |      | |_|____________________Rest of payload
|      | |      | |      | |      | |______|________________________2nd payload byte
|      | |      | |      | |______|_________________________________1st payload byte
|      | |      | |______|__________________________________________# of payload bytes
|      | |______|___________________________________________________COBS Overhead byte
|______|____________________________________________________________Start byte (constant)
```

# **How To Use:**

# *Both Arduinos:*
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


# *Transmitting Arduino:*
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

# *Complete TX Code:*
```c++
#include "SerialTransfer.h"

SerialTransfer myTransfer;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);
}

void loop()
{
  myTransfer.txBuff[0] = 'h';
  myTransfer.txBuff[1] = 'i';
  myTransfer.txBuff[2] = '\n';
  
  myTransfer.sendData(3);
  delay(100);
}
```


# *Receiving Arduino:*
1.) Repetitively check to see if a new packet has been completely received. Also, check to see if any transfer errors have occurred:
```c++
if(myTransfer.available())
{
  // see next step
}
else if(myTransfer.status < 0)
{
  Serial.print("ERROR: ");
  Serial.println(myTransfer.status);
}
```

2.) If a full packet has been received, use the SerialTransfer class's RX buffer to access the received data bytes:
```c++
Serial.println("New Data");
for(byte i = 0; i < myTransfer.bytesRead; i++)
  Serial.write(myTransfer.rxBuff[i]);
Serial.println();
```

# *Complete RX Code:*
```c++
#include "SerialTransfer.h"

SerialTransfer myTransfer;

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);
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
  else if(myTransfer.status < 0)
  {
    Serial.print("ERROR: ");
    Serial.println(myTransfer.status);
  }
}
```
