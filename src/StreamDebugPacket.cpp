#include "StreamDebugPacket.h"

StreamDebugPacket::StreamDebugPacket(Stream* debugPort) : debugPort(debugPort), Packet(debugPort != nullptr)
{
	// need to give the debug port a kick to get things working for some strange reason...
	debugPort->println();
}

void StreamDebugPacket::printDebug(const char* msg)
{
	if (debugPort == nullptr)
		return;

	debugPort->println(msg);
}
