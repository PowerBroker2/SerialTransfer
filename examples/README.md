# SerialTransfer examples

The curated examples are grouped by physical interface with `Serial_` and
`I2C_` name prefixes. Each sketch folder contains one Arduino IDE sketch whose
filename matches the folder name.

## Serial

| Example | What it demonstrates | Setup |
| --- | --- | --- |
| [Serial_MinimalLoopback](Serial_MinimalLoopback/Serial_MinimalLoopback.ino) | Sends a counter and verifies its echo. | Uses `Serial1` with a TX1-to-RX1 jumper when available. Otherwise it uses `SoftwareSerial` at 9600 baud; upload the same sketch to a second board with `SERIALTRANSFER_SOFTWARE_SERIAL_PEER` set to `1`, cross-connect TX/RX, and share ground. |
| [Serial_Callbacks](Serial_Callbacks/Serial_Callbacks.ino) | Dispatches different callbacks by packet ID. | Jumper TX1 to RX1; open the USB Serial monitor at 115200 baud. |
| [Serial_LoopbackStress](Serial_LoopbackStress/Serial_LoopbackStress.ino) | Measures 255-byte packet throughput, verified goodput, latency, and parser errors. | Designed for Teensy 4.x `Serial1` with a TX1-to-RX1 jumper; defaults to 115200 baud and a 10-second run. Other boards need an RX ring buffer large enough to hold the incoming frame while the transmit call completes. |
| [Serial_FileTransferSender](Serial_FileTransferSender/Serial_FileTransferSender.ino) and [Serial_FileTransferReceiver](Serial_FileTransferReceiver/Serial_FileTransferReceiver.ino) | Transfers a 512-byte generated file in acknowledged chunks and verifies the complete-file FNV-1a checksum. | Upload one sketch to each board. Cross-connect TX1/RX1 in both directions and connect grounds. |

## I²C

I²C uses a master and a slave board; wiring SDA and SCL back to the same board
does not create a useful loopback test.

| Example | What it demonstrates | Setup |
| --- | --- | --- |
| [I2C_MinimalTwoBoard](I2C_MinimalTwoBoard/I2C_MinimalTwoBoard.ino) | Sends a counter from an I²C master to a slave. | Upload to both boards; set `I2C_TEST_MASTER` to `1` on the master and `0` on the slave. Slave address is `0x42`. |
| [I2C_Callbacks](I2C_Callbacks/I2C_Callbacks.ino) | Dispatches callbacks by packet ID from normal loop context. | Same two-board setup and role switch as I2C_MinimalTwoBoard. |
| [I2C_LoopbackStress](I2C_LoopbackStress/I2C_LoopbackStress.ino) | Measures full-size packet round-trip latency, estimated bus rate, verified payload goodput, and parser/sequence errors. | Upload to both boards; set the role macro. The master prints the benchmark, the slave prints receive analysis. Defaults to 100 kHz, 255-byte payloads, and 10 seconds. |
| [I2C_FileTransferMaster](I2C_FileTransferMaster/I2C_FileTransferMaster.ino) and [I2C_FileTransferSlave](I2C_FileTransferSlave/I2C_FileTransferSlave.ino) | Transfers a 512-byte generated file in acknowledged chunks and verifies its complete-file checksum. | Upload one sketch to each board. The slave address is `0x42`; connect SDA, SCL, and ground. |

The I²C file-transfer example uses SerialTransfer frames for file data and a
small master-requested I²C response for chunk acknowledgements. A master must
initiate every I²C read; a slave cannot transmit spontaneously.

## General notes

- Open debug monitors at 115200 baud unless a sketch says otherwise.
- `Serial_MinimalLoopback` selects `SoftwareSerial` when the board core does not expose
  the common `Serial1` feature macros. Set `SERIALTRANSFER_USE_SOFTWARE_SERIAL`
  to `0` or `1` at the top of the sketch to override detection.
- Packet IDs select callback-array entries. Null callback entries are skipped;
  callback arrays can cover IDs 0 through 255.
- I²C frames are split into consecutive transactions to fit common 32-byte
  `Wire` buffers. Define `SERIALTRANSFER_I2C_CHUNK_SIZE` before including
  `I2CTransfer.h` only when the board's Wire buffer capacity differs.
- The file-transfer sketches generate a deterministic 512-byte test file in
  code, so they need no SD card. Replace the sample byte source with a storage
  read when adapting them to a real file.
- The old flat sketches are preserved under [Legacy](Legacy/). Use the curated
  sketches above for the current packet format.
