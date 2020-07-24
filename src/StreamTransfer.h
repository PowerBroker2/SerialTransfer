#pragma once

#include <Stream.h>

#include "Packet.h"


class StreamTransfer : public Packet
{
  public: // <<---------------------------------------//public
	StreamTransfer(Stream& port);
	StreamTransfer(Stream& port, Stream& debugPort);

  protected: // <<---------------------------------------//protected
	// Vritual functions to override
	virtual bool    bytesAvailable();
	virtual uint8_t readByte();
	virtual void    writeBytes();
	virtual void    printDebug(const char* msg);

  private: // <<---------------------------------------//private
	Stream& port;
	Stream* debugPort;
};
