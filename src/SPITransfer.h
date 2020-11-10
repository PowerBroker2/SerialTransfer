#pragma once
/* UNCOMMENT FOR USAGE
#include "Arduino.h"

#if not(defined(MBED_H) || defined(__SAM3X8E__)) // These boards are/will not be supported by SPITransfer.h

#include "Packet.h"
#include "SPI.h"


class SPITransfer
{
  public: // <<---------------------------------------//public
	Packet  packet;
	uint8_t bytesRead = 0;
	int8_t  status    = 0;


	void    begin(SPIClass& _port, const configST configs, const uint8_t& _SS = SS);
	void    begin(SPIClass& _port, const uint8_t& _SS = SS, const bool _debug = true, Stream& _debugPort = Serial);
	uint8_t sendData(const uint16_t& messageLen, const uint8_t packetID = 0);
	uint8_t available();
	uint8_t currentPacketID();

*/
	/*
	 uint16_t SPITransfer::txObj(const T &val, const uint16_t &index=0, const uint16_t &len=sizeof(T))
	 Description:
	 ------------
	  * Stuffs "len" number of bytes of an arbitrary object (byte, int,
	  float, double, struct, etc...) into the transmit buffer (txBuff)
	  starting at the index as specified by the argument "index"
	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied to the
	  transmit buffer (txBuff)
	  * const uint16_t &index - Starting index of the object within the
	  transmit buffer (txBuff)
	  * const uint16_t &len - Number of bytes of the object "val" to transmit
	 Return:
	 -------
	  * uint16_t maxIndex - uint16_t maxIndex - Index of the transmit buffer (txBuff) that directly follows the bytes processed
	  by the calling of this member function
	*//*
	template <typename T>
	uint16_t txObj(const T& val, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		return packet.txObj(val, index, len);
	}

*/
	/*
	 uint16_t SPITransfer::rxObj(const T &val, const uint16_t &index=0, const uint16_t &len=sizeof(T))
	 Description:
	 ------------
	  * Reads "len" number of bytes from the receive buffer (rxBuff)
	  starting at the index as specified by the argument "index"
	  into an arbitrary object (byte, int, float, double, struct, etc...)
	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied into from the
	  receive buffer (rxBuff)
	  * const uint16_t &index - Starting index of the object within the
	  receive buffer (rxBuff)
	  * const uint16_t &len - Number of bytes in the object "val" received
	 Return:
	 -------
	  * uint16_t maxIndex - Index of the receive buffer (rxBuff) that directly follows the bytes processed
	  by the calling of this member function
	*//*
	template <typename T>
	uint16_t rxObj(const T& val, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		return packet.rxObj(val, index, len);
	}

*/
	/*
	 uint8_t SPITransfer::sendDatum(const T &val, const uint16_t &len=sizeof(T))
	 Description:
	 ------------
	  * Stuffs "len" number of bytes of an arbitrary object (byte, int,
	  float, double, struct, etc...) into the transmit buffer (txBuff)
	  starting at the index as specified by the argument "index" and
	  automatically transmits the bytes in an individual packet
	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied to the
	  transmit buffer (txBuff)
	  * const uint16_t &len - Number of bytes of the object "val" to transmit
	 Return:
	 -------
	  * uint8_t - Number of payload bytes included in packet
	*//*
	template <typename T>
	uint8_t sendDatum(const T& val, const uint16_t& len = sizeof(T))
	{
		return sendData(packet.txObj(val, 0, len));
	}


  private: // <<---------------------------------------//private
	SPIClass* port;
	uint8_t   ssPin;
};

#endif // not (defined(MBED_H) || defined(__SAM3X8E__))
*/
