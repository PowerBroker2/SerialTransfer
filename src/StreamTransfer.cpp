#include "StreamTransfer.h"

StreamTransfer::StreamTransfer(Stream& port, Stream* debugPort) : port(port), StreamDebugPacket(debugPort)
{
}


StreamTransfer::StreamTransfer(Stream& port, Stream& debugPort) : port(port), StreamDebugPacket(&debugPort)
{
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
