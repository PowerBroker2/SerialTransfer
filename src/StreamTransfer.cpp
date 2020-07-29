#include "StreamTransfer.h"

StreamTransfer::StreamTransfer()
{
}


StreamTransfer::StreamTransfer(Stream& port, Stream* debugPort)
{
	begin(port, debugPort);
}


StreamTransfer::StreamTransfer(Stream& port, Stream& debugPort)
{
	begin(port, debugPort);
}


void StreamTransfer::begin(Stream& port, Stream* debugPort)
{
	this->port = &port;

	StreamDebugPacket::begin(debugPort);
}


void StreamTransfer::begin(Stream& port, Stream& debugPort)
{
	begin(port, &debugPort);
}


bool StreamTransfer::bytesAvailable()
{
	return port->available();
}


uint8_t StreamTransfer::readByte()
{
	return port->read();
}


void StreamTransfer::writeBytes()
{
	port->write(txRawBuff, PREAMBLE_SIZE + bytesToSend + POSTAMBLE_SIZE);
}
