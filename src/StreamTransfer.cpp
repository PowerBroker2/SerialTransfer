#include "StreamTransfer.h"

StreamTransfer::StreamTransfer(Stream& port) : port(port), debugPort(nullptr), Packet(false)
{
}

StreamTransfer::StreamTransfer(Stream& port, Stream& debugPort) : port(port), debugPort(&debugPort), Packet(true)
{
	// need to give the debug port a kick to get things working for some strange reason...
	debugPort.println();
}

bool StreamTransfer::bytesAvailable()
{
	return port.available();
}

uint8_t StreamTransfer::readByte()
{
	return port.read();
}

void StreamTransfer::writeBytes()
{
	port.write(preamble, PREAMBLE_SIZE);
	port.write(txBuff, bytesToSend);
	port.write(postamble, POSTAMBLE_SIZE);
}

void StreamTransfer::printDebug(const char* msg)
{
	if (debugPort == nullptr)
		return;

	debugPort->println(msg);
}
