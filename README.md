<p align="center">
  <img width="475" height="184" src="https://raw.githubusercontent.com/PowerBroker2/SerialTransfer/master/logo.png">
</p>

# SerialTransfer
[![GitHub version](https://badge.fury.io/gh/PowerBroker2%2FSerialTransfer.svg)](https://badge.fury.io/gh/PowerBroker2%2FSerialTransfer)  [![arduino-library-badge](https://www.ardu-badge.com/badge/SerialTransfer.svg?)](https://www.ardu-badge.com/SerialTransfer)<br /><br />
SerialTransfer is an easy to use Arduino library for transferring packetized data fast and reliably via **Serial, I2C, and SPI interfaces**. Interconnect your Arduinos today minus the headache with SerialTransfer!

[A mirror library is also available for Python!](https://github.com/PowerBroker2/pySerialTransfer)

This library:
- can be downloaded via the Arduino IDE's Libraries Manager (search "SerialTransfer.h")
- works with "software-serial", "software-I2C", and "software-SPI" libraries
- is non blocking
- uses a start byte and payload length for packet framing
- uses CRC-8 (Polynomial 0x9B with a lookup table) over the packet ID, length, and payload
- allows dynamically sized packets with payloads from 0 to 255 bytes
- supports user-specified callback functions
- **can transfer bytes, ints, floats, structs, even large files like JPEGs and CSVs!!**

# Packet Anatomy:
```
START | ID | LENGTH | PAYLOAD (0..255 bytes) | CRC-8(ID, LENGTH, PAYLOAD)
```

I2C frames are split into consecutive transactions of up to 32 bytes by default
so the maximum-size packet fits common `Wire` buffers. The receiver reassembles
those chunks in the packet parser. Call `I2CTransfer::available()` or
`I2CTransfer::tick()` regularly from `loop()`; configured packet callbacks are
dispatched there, outside the Wire receive interrupt. The receiver has one
packet buffer, so consume each packet before the sender sends another. Define
`SERIALTRANSFER_I2C_CHUNK_SIZE` before including `I2CTransfer.h` to match a
different `Wire` buffer capacity.

# ***How To Use:***

See the [curated example guide](examples/README.md) for Serial and I²C quickstarts,
loopback analysis, callbacks, and multi-packet file transfer. Older sketches are
retained in `examples/Legacy/`.

- datum = tx/rx a single object
- data = tx/rx multiple objects

For object helpers, the library copies raw in-memory bytes. Structure padding,
endianness, and floating-point formats are not converted; use matching layouts
or serialize multi-byte fields explicitly when communicating across different
architectures.

# ***NOTE:***

SPITransfer.h and it's associated features are not supported for the Arduino Nano 33 BLE or DUE and other boards. This header file is disabled by default, but can be enabled by commenting out `#define DISABLE_SPI_SERIALTRANSFER 1` within `SerialTransfer.h`.
