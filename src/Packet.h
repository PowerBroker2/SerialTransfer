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
#include "Arduino.h"
#include "PacketCRC.h"


typedef void (*functionPtr)();


const int8_t CONTINUE           = 3;
const int8_t NEW_DATA           = 2;
const int8_t NO_DATA            = 1;
const int8_t CRC_ERROR          = 0;
const int8_t PAYLOAD_ERROR      = -1;
const int8_t STOP_BYTE_ERROR    = -2;
const int8_t STALE_PACKET_ERROR = -3;

const uint8_t START_BYTE = 0x7E;
const uint8_t STOP_BYTE  = 0x81;

const uint8_t PREAMBLE_SIZE   = 4;
const uint8_t POSTAMBLE_SIZE  = 2;
const uint8_t MAX_PACKET_SIZE = 0xFE; // Maximum allowed payload bytes per packet

const uint8_t DEFAULT_TIMEOUT = 50;


struct configST
{
	Stream*            debugPort    = &Serial;
	bool               debug        = true;
	const functionPtr* callbacks    = NULL;
	uint8_t            callbacksLen = 0;
	uint32_t           timeout      = __UINT32_MAX__;
};


class Packet
{
  public: // <<---------------------------------------//public
	uint8_t txBuff[MAX_PACKET_SIZE];
	uint8_t rxBuff[MAX_PACKET_SIZE];
	uint8_t preamble[PREAMBLE_SIZE]   = {START_BYTE, 0, 0, 0};
	uint8_t postamble[POSTAMBLE_SIZE] = {0, STOP_BYTE};

	uint8_t bytesRead = 0;
	int8_t  status    = 0;


	void    begin(const configST& configs);
	void    begin(const bool& _debug = true, Stream& _debugPort = Serial, const uint32_t& _timeout = DEFAULT_TIMEOUT);
	uint8_t constructPacket(const uint16_t& messageLen, const uint8_t& packetID = 0);
	uint8_t parse(const uint8_t& recChar, const bool& valid = true);
	uint8_t currentPacketID();
	void    reset();


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
	uint16_t txObj(const T& val, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		uint8_t* ptr = (uint8_t*)&val;
		uint16_t maxIndex;

		if ((len + index) > MAX_PACKET_SIZE)
			maxIndex = MAX_PACKET_SIZE;
		else
			maxIndex = len + index;

		for (uint16_t i = index; i < maxIndex; i++)
		{
			txBuff[i] = *ptr;
			ptr++;
		}

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
	uint16_t rxObj(const T& val, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		uint8_t* ptr = (uint8_t*)&val;
		uint16_t maxIndex;

		if ((len + index) > MAX_PACKET_SIZE)
			maxIndex = MAX_PACKET_SIZE;
		else
			maxIndex = len + index;

		for (uint16_t i = index; i < maxIndex; i++)
		{
			*ptr = rxBuff[i];
			ptr++;
		}

		return maxIndex;
	}


  private: // <<---------------------------------------//private
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
	fsm state = find_start_byte;

	const functionPtr* callbacks    = NULL;
	uint8_t            callbacksLen = 0;

	Stream* debugPort;
	bool    debug = false;

	uint8_t bytesToRec      = 0;
	uint8_t payIndex        = 0;
	uint8_t idByte          = 0;
	uint8_t overheadByte    = 0;
	uint8_t recOverheadByte = 0;

	uint32_t packetStart    = 0;
	uint32_t timeout;


	void    calcOverhead(uint8_t arr[], const uint8_t& len);
	int16_t findLast(uint8_t arr[], const uint8_t& len);
	void    stuffPacket(uint8_t arr[], const uint8_t& len);
	void    unpackPacket(uint8_t arr[]);
};
