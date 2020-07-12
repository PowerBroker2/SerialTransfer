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

# ***How To Use:***
# *Initialization:*

There are two ways to initialize all transfer classes:

1.) Standard Method:
```c++
void begin(Stream &_port, const bool _debug=true, Stream &_debugPort=Serial);
void begin(TwoWire &_port, const bool _debug=true, Stream &_debugPort=Serial);
void begin(SPIClass &_port, const bool _debug=true, Stream &_debugPort=Serial);
```

**Note:** You can't use the standard method for I2C receiver code - you must use the advanced method for class initialization.

Example:
```c++
myTransfer.begin(Serial1);
```

2.) Advanced Method:
```c++
void begin(Stream &_port, const configST configs);
void begin(TwoWire &_port, const configST configs);
void begin(SPIClass &_port, const configST configs);
```

This method is mainly used to specify callback functions that are called when packets with specific IDs are successfully parsed.

**Note:** You must use the advanced method for I2C class initialization.

Example:
```c++
functionPtr callbackArr[] = { hi }; // Callback function (`hi()`) is defined before `setup()`

///////////////////////////////////////////////////////////////// Config Parameters
configST myConfig;
myConfig.debug        = true;
myConfig.callbacks    = callbackArr;
myConfig.callbacksLen = sizeof(callbackArr) / sizeof(functionPtr);
/////////////////////////////////////////////////////////////////

myTransfer.begin(Serial1, myConfig);
```

# *Transmitting Data:*

The transmission of data within this library consists of two stages:

1.) Stuffing the library's transmit buffer:

- Bytes can be automatically stuffed into the transmit buffer using the `txObj()` method:
  - `txObj()` stuffs all static typed objects (pretty much everything except "S"trings)

- Bytes can be manually stuffed into the transmit buffer by doing the following:

| Hardware Interface | Transmit Buffer |
| --- | --- |
| UART | `<SerialTransfer instance>.packet.txBuff` |
| I2C | `<I2CTransfer instance>.packet.txBuff` |
| SPI | `<SPITransfer instance>.packet.txBuff` |
