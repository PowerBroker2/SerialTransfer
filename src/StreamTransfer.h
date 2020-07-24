#pragma once

#include "StreamDebugPacket.h"


class StreamTransfer : public StreamDebugPacket
{
  public: // <<---------------------------------------//public
	StreamTransfer(Stream& port, Stream* debugPort = nullptr);
	StreamTransfer(Stream& port, Stream& debugPort);

  protected: // <<---------------------------------------//protected
	// Vritual functions to override
	virtual bool    bytesAvailable();
	virtual uint8_t readByte();
	virtual void    writeBytes();

  private: // <<---------------------------------------//private
	Stream& port;
};
