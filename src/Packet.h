/*
01111110 00000000 11111111 00000000 00000000 00000000 ... 00000000 10000001
|      | |      | |      | |      | |      | |      | | | |      | |______|__Stop byte
|      | |      | |      | |      | |      | |      | | | |______|___________8-bit CRC
|      | |      | |      | |      | |      | |      | |_|____________________Rest of payload
|      | |      | |      | |      | |      | |______|________________________2nd payload byte
|      | |      | |      | |      | |______|_________________________________1st payload byte
|      | |      | |      | |______|__________________________________________# of payload bytes
|      | |      | |______|___________________________________________________COBS Overhead byte
|      | |______|____________________________________________________________Packet ID (0 by default)
|______|_____________________________________________________________________Start byte (constant)
*/

#pragma once

#include <stdint.h>

#include "PacketCRC.h"


typedef void (*functionPtr)();


enum ParserState : int8_t
{
	CONTINUE        = 2,
	NEW_DATA        = 1,
	NO_DATA         = 0,
	CRC_ERROR       = -1,
	PAYLOAD_ERROR   = -2,
	STOP_BYTE_ERROR = -3
};

constexpr uint8_t START_BYTE = 0x7E;
constexpr uint8_t STOP_BYTE  = 0x81;

constexpr uint8_t PREAMBLE_SIZE   = 4;
constexpr uint8_t POSTAMBLE_SIZE  = 2;
constexpr uint8_t MAX_PACKET_SIZE = 0xFE; // Maximum allowed payload bytes per packet


class Packet
{
  public: // <<---------------------------------------//public
	uint8_t txBuff[MAX_PACKET_SIZE];
	uint8_t rxBuff[MAX_PACKET_SIZE];

	uint8_t bytesToSend = 0;
	uint8_t bytesRead   = 0;

  protected: // <<---------------------------------------//protected
	Packet(bool debug = false);

	// Vritual functions to override
	virtual bool    bytesAvailable() = 0;
	virtual uint8_t readByte()       = 0;
	virtual void    writeBytes()     = 0;
	virtual void    printDebug(const char* msg);

  public: // <<---------------------------------------//public
	uint8_t     sendPacket(uint8_t packetID = 0);
	uint8_t     available();
	uint8_t     currentPacketID();
	ParserState getStatus();


	/*
	 uint16_t Packet::txObj(const T &val, const uint16_t &index=0, const uint16_t &len=sizeof(T))
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
	*/
	template <typename T>
	uint16_t txObj(const T& val, uint8_t index = 0xFF, size_t len = sizeof(T))
	{
		const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&val);
		uint8_t        maxIndex;

		if (index == 0xFF)
			index = bytesToSend;

		if ((len + index) > MAX_PACKET_SIZE)
			maxIndex = MAX_PACKET_SIZE;
		else
			maxIndex = len + index;

		for (uint8_t i = index; i < maxIndex; i++)
			txBuff[i] = *(ptr++);

		if (bytesToSend < maxIndex)
			bytesToSend = maxIndex;

		return maxIndex;
	}


	/*
	 uint16_t Packet::rxObj(const T &val, const uint16_t &index=0, const uint16_t &len=sizeof(T))
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
	*/
	template <typename T>
	uint16_t rxObj(T& val, uint8_t index = 0xFF, size_t len = sizeof(T))
	{
		uint8_t* ptr = reinterpret_cast<uint8_t*>(&val);
		uint8_t  maxIndex;

		if (index == 0xFF)
			index = bytesRead;

		if ((len + index) > MAX_PACKET_SIZE)
			maxIndex = MAX_PACKET_SIZE;
		else
			maxIndex = len + index;

		for (uint16_t i = index; i < maxIndex; i++)
			*(ptr++) = rxBuff[i];

		if (bytesRead < maxIndex)
			bytesRead = maxIndex;

		return maxIndex;
	}


  private: // <<---------------------------------------//private
	bool debug;

	uint8_t preamble[PREAMBLE_SIZE]   = {START_BYTE, 0, 0, 0};
	uint8_t postamble[POSTAMBLE_SIZE] = {0, STOP_BYTE};

	functionPtr* callbacks    = NULL;
	uint8_t      callbacksLen = 0;

	// Parser State
	enum fsm
	{
		find_start_byte,
		find_id_byte,
		find_overhead_byte,
		find_payload_len,
		find_payload,
		find_crc,
		find_end_byte
	};

	ParserState status          = NO_DATA;
	fsm         state           = find_start_byte;
	uint8_t     bytesRec        = 0;
	uint8_t     bytesToRec      = 0;
	uint8_t     payIndex        = 0;
	uint8_t     idByte          = 0;
	uint8_t     recOverheadByte = 0;


	uint8_t stuffPacket();
	void    unpackPacket();
};
