#pragma once

#include <Arduino.h>

#include "Packet.h"


class SerialTransfer : public Packet
{
  public: // <<---------------------------------------//public
	SerialTransfer(Stream& port);
	SerialTransfer(Stream& port, Stream& debugPort);

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
