#pragma once

#include <Stream.h>

#include "Packet.h"


class StreamDebugPacket : public Packet
{
  protected: // <<---------------------------------------//protected
	StreamDebugPacket(Stream* debugPort = nullptr);

	virtual void printDebug(const char* msg);

  private: // <<---------------------------------------//private
	Stream* debugPort;
};
