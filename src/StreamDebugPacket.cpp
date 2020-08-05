#include "StreamDebugPacket.h"

StreamDebugPacket::StreamDebugPacket()
{
}


StreamDebugPacket::StreamDebugPacket(Stream* debugPort)
{
	begin(debugPort);
}


void StreamDebugPacket::begin(Stream* debugPort)
{
	this->debugPort = debugPort;
	// need to give the debug port a kick to get things working for some strange reason...
	debugPort->println();

	Packet::begin(debugPort != nullptr);
}


void StreamDebugPacket::printDebug(const char* msg) const
{
	if (debugPort == nullptr)
		return;

	debugPort->println(msg);
}
