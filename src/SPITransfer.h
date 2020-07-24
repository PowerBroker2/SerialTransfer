#pragma once

#include <Arduino.h>

// These boards are/will not be supported by SPITransfer.h
#if !(defined(MBED_H) || defined(__SAM3X8E__))

#include <SPI.h>

#include "StreamDebugPacket.h"


class SPITransfer : public StreamDebugPacket
{
  public: // <<---------------------------------------//public
	SPITransfer(SPIClass& port, uint8_t ssPin = SS, Stream* debugPort = nullptr);
	SPITransfer(SPIClass& port, Stream& debugPort, uint8_t ssPin = SS);

  protected: // <<---------------------------------------//protected
	// Vritual functions to override
	virtual bool    bytesAvailable();
	virtual uint8_t readByte();
	virtual void    writeBytes();

  private: // <<---------------------------------------//private
	SPIClass&     port;
	const uint8_t ssPin;
	uint8_t       recByte;
};

#endif // !(defined(MBED_H) || defined(__SAM3X8E__))
