#if !(defined(MBED_H) || defined(__SAM3X8E__)) // These boards are/will not be supported by SPITransfer.h

#include "SPITransfer.h"


SPITransfer::SPITransfer(SPIClass& port, uint8_t ssPin, Stream* debugPort) : port(port), ssPin(ssPin), StreamDebugPacket(debugPort)
{
}


SPITransfer::SPITransfer(SPIClass& port, Stream& debugPort, uint8_t ssPin) : port(port), ssPin(ssPin), StreamDebugPacket(&debugPort)
{
}


bool SPITransfer::bytesAvailable()
{
	uint8_t read = port.transfer(0x00);

	return (state != fsm::find_start_byte) || (read == START_BYTE);
}


uint8_t SPITransfer::readByte()
{
	return SPDR;
}


void SPITransfer::writeBytes()
{
	digitalWrite(ssPin, LOW); // Enable SS (active low)
	delayMicroseconds(1);

	port.transfer(preamble, PREAMBLE_SIZE);
	port.transfer(txBuff, bytesToSend);
	port.transfer(postamble, POSTAMBLE_SIZE);

	digitalWrite(ssPin, HIGH); // Disable SS (active low)
}

#endif // !(defined(MBED_H) || defined(__SAM3X8E__))
