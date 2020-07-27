#if !(defined(MBED_H) || defined(__SAM3X8E__)) // These boards are/will not be supported by SPITransfer.h

#include "SPITransfer.h"


SPITransfer::SPITransfer()
{
}


SPITransfer::SPITransfer(SPIClass& port, uint8_t ssPin, Stream* debugPort)
{
	begin(port, ssPin, debugPort);
}


SPITransfer::SPITransfer(SPIClass& port, Stream& debugPort, uint8_t ssPin)
{
	begin(port, debugPort, ssPin);
}


void SPITransfer::begin(SPIClass& port, uint8_t ssPin, Stream* debugPort)
{
	this->port  = &port;
	this->ssPin = ssPin;

	StreamDebugPacket::begin(debugPort);
}


void SPITransfer::begin(SPIClass& port, Stream& debugPort, uint8_t ssPin)
{
	begin(port, &debugPort, ssPin);
}


bool SPITransfer::bytesAvailable()
{
	recByte = port->transfer(0x00);

	return (state != fsm::find_start_byte) || (recByte == START_BYTE);
}


uint8_t SPITransfer::readByte()
{
	return recByte;
}


void SPITransfer::writeBytes()
{
	digitalWrite(ssPin, LOW); // Enable SS (active low)
	delayMicroseconds(1);

	port->transfer(preamble, PREAMBLE_SIZE);
	port->transfer(txBuff, bytesToSend);
	port->transfer(postamble, POSTAMBLE_SIZE);

	digitalWrite(ssPin, HIGH); // Disable SS (active low)
}

#endif // !(defined(MBED_H) || defined(__SAM3X8E__))
