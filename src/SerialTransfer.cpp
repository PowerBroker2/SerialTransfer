#include "SerialTransfer.h"

SerialTransfer::SerialTransfer(Stream& port) : port(port), debugPort(nullptr), Packet(false)
{
}

SerialTransfer::SerialTransfer(Stream& port, Stream& debugPort) : port(port), debugPort(&debugPort), Packet(true)
{
	// need to give the debug port a kick to get things working for some strange reason...
	debugPort.println();
}

bool SerialTransfer::bytesAvailable()
{
	return port.available();
}

uint8_t SerialTransfer::readByte()
{
	return port.read();
}

void SerialTransfer::writeBytes()
{
	port.write(preamble, PREAMBLE_SIZE);
	port.write(txBuff, bytesToSend);
	port.write(postamble, POSTAMBLE_SIZE);
}

void SerialTransfer::printDebug(const char* msg)
{
	if (debugPort == nullptr)
		return;

	debugPort->println(msg);
}
