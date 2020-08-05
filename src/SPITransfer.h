#pragma once

#include <Arduino.h>

// You can forcefully enable or disable SPITransfer by defining this to 1 or 0 respectively
#ifndef SPI_TRANSFER
// These boards are/will not be supported by SPITransfer.h
#if defined(MBED_H) || defined(__SAM3X8E__) || defined(ESP8266)
#define SPI_TRANSFER 0
#else
#define SPI_TRANSFER 1
#endif
#endif // ndef SPI_TRANSFER

#if SPI_TRANSFER

#include <SPI.h>

#include "StreamDebugPacket.h"


class SPITransfer : public StreamDebugPacket
{
  public: // <<---------------------------------------//public
	SPITransfer();
	SPITransfer(SPIClass& port, uint8_t ssPin = SS, Stream* debugPort = nullptr);
	SPITransfer(SPIClass& port, Stream& debugPort, uint8_t ssPin = SS);

	void begin(SPIClass& port, uint8_t ssPin = SS, Stream* debugPort = nullptr);
	void begin(SPIClass& port, Stream& debugPort, uint8_t ssPin = SS);

  protected: // <<---------------------------------------//protected
	// Vritual functions to override
	virtual bool    bytesAvailable();
	virtual uint8_t readByte();
	virtual void    writeBytes();

  private: // <<---------------------------------------//private
	SPIClass* port;
	uint8_t   ssPin;
	uint8_t   recByte;
};

#endif // SPI_TRANSFER
