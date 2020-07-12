# SerialTransfer
[![GitHub version](https://badge.fury.io/gh/PowerBroker2%2FSerialTransfer.svg)](https://badge.fury.io/gh/PowerBroker2%2FSerialTransfer)  [![arduino-library-badge](https://www.ardu-badge.com/badge/SerialTransfer.svg?)](https://www.ardu-badge.com/SerialTransfer)<br /><br />
Arduino library (available through the Arduino IDE's Libraries Manager!) to transfer packetized data fast/reliably via Serial, I2C, and SPI. [A mirror library is also available for Python!](https://github.com/PowerBroker2/pySerialTransfer)

This library:
- can be downloaded via the Arduino IDE's Libraries Manager (search "SerialTransfer.h")
- works with "software-serial" libraries
- is non blocking
- uses packet delimiters
- uses consistent overhead byte stuffing
- uses CRC-8 (Polynomial 0x9B with lookup table)
- allows the use of dynamically sized packets (packets can have payload lengths anywhere from 1 to 254 bytes)
- supports user-specified callback functions
- **can transfer bytes, ints, floats, structs, even large files like [JPEGs](https://forum.arduino.cc/index.php?topic=676260.msg4567624#msg4567624)!!**

# Packet Anatomy:
```
01111110 00000000 11111111 00000000 00000000 00000000 ... 00000000 10000001
|      | |      | |      | |      | |      | |      | | | |      | |______|__Stop byte
|      | |      | |      | |      | |      | |      | | | |______|___________8-bit CRC
|      | |      | |      | |      | |      | |      | |_|____________________Rest of payload
|      | |      | |      | |      | |      | |______|________________________2nd payload byte
|      | |      | |      | |      | |______|_________________________________1st payload byte
|      | |      | |      | |______|__________________________________________# of payload bytes
|      | |      | |______|___________________________________________________COBS Overhead byte
|      | |______|____________________________________________________________Packet ID (0 by default)
|______|_____________________________________________________________________Start byte (constant)
```

# **How To Use:**

# *Both Arduinos:*
1.) Include the library and create a transfer class instance:
```c++
#include "SerialTransfer.h"
SerialTransfer myTransfer;
```

2.) Create data objects to be transferred:
```c++
struct STRUCT {
  char z;
  float y;
} testStruct;

char arr[] = "hello";
```

3.) Determine which serial ports the transfer will occur on and initialize it:
```c++
Serial1.begin(115200);
```

4.) Pass the SerialX class to the SerialTransfer class for initialization:
```c++
myTransfer.begin(Serial1);
```


# *Transmitting Arduino:*
1.) Insert data objects into the SerialTransfer TX buffer using `myTransfer.txObj()`:
```c++
// use this variable to keep track of how many
// bytes we're stuffing in the transmit buffer
uint16_t sendSize = 0;

///////////////////////////////////////// Stuff buffer with struct
sendSize = myTransfer.txObj(testStruct, sendSize);

///////////////////////////////////////// Stuff buffer with array
sendSize = myTransfer.txObj(arr, sendSize);
```

2.) Transmit the data via the "sendData" member function. The argument of sendData() is the number of bytes of the TX buffer to be transmitted:
```c++
myTransfer.sendData(sendSize);
```

# *Complete TX Code:*
```c++
#include "SerialTransfer.h"


SerialTransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;

char arr[] = "hello";


void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  myTransfer.begin(Serial1);

  testStruct.z = '$';
  testStruct.y = 4.5;
}


void loop()
{
  // use this variable to keep track of how many
  // bytes we're stuffing in the transmit buffer
  uint16_t sendSize = 0;

  ///////////////////////////////////////// Stuff buffer with struct
  sendSize = myTransfer.txObj(testStruct, sendSize);

  ///////////////////////////////////////// Stuff buffer with array
  sendSize = myTransfer.txObj(arr, sendSize);

  ///////////////////////////////////////// Send buffer
  myTransfer.sendData(sendSize);
  delay(500);
}
```


# *Receiving Arduino:*
1.) Repetitively check to see if a new packet has been completely received:
```c++
if(myTransfer.available())
{
  // New packet detected!
  // See next step
}
```

2.) If a full packet has been received, use the SerialTransfer class's RX buffer to manually (using `myTransfer.bytesRead`) and/or automatically (using `myTransfer.rxObj()`) access the received data bytes:
```c++
// use this variable to keep track of how many
// bytes we've processed from the receive buffer
uint16_t recSize = 0;

recSize = myTransfer.rxObj(testStruct, recSize);
Serial.print(testStruct.z);
Serial.print(testStruct.y);
Serial.print(" | ");

recSize = myTransfer.rxObj(arr, recSize);
Serial.println(arr);
```

# *Complete RX Code:*
```c++
#include "SerialTransfer.h"


SerialTransfer myTransfer;

struct STRUCT {
  char z;
  float y;
} testStruct;

char arr[6];


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
}
```
