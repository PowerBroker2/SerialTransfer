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
#include "ST_CRC.h"




typedef char*(*functionPtr)();




const int8_t CONTINUE         = 3;
const int8_t NEW_DATA         = 2;
const int8_t NO_DATA          = 1;
const int8_t CRC_ERROR        = 0;
const int8_t PAYLOAD_ERROR    = -1;
const int8_t STOP_BYTE_ERROR  = -2;

const uint8_t START_BYTE      = 0x7E;
const uint8_t STOP_BYTE       = 0x81;

const uint8_t MAX_PACKET_SIZE = 0xFE;




struct configST {
	Stream* port           = &Serial;
	Stream* debugPort      = &Serial;
	bool debug             = true;
	functionPtr* callbacks = NULL;
	uint8_t callbacksLen   = 0;
};




class SerialTransfer
{
public: // <<---------------------------------------//public
	SerialTransferCRC crc;

	uint8_t txBuff[MAX_PACKET_SIZE];
	uint8_t rxBuff[MAX_PACKET_SIZE];

	uint8_t bytesRead = 0;
	int8_t status = 0;




	void begin(const configST configs);
	void begin(Stream &_port, const bool _debug=true, Stream &_debugPort=Serial);
	uint8_t sendData(const uint16_t &messageLen, const uint8_t packetID=0);
	uint8_t available();
	bool tick();
	uint8_t currentPacketID();




	/*
	 void SerialTransfer::txObj(const T &val, const uint16_t &len=sizeof(T), const uint16_t &index=0)
	 Description:
	 ------------
	  * Stuffs "len" number of bytes of an arbitrary object (byte, int,
	  float, double, struct, etc...) into the transmit buffer (txBuff)
	  starting at the index as specified by the argument "index"

	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied to the
	  transmit buffer (txBuff)
	  * const uint16_t &len - Number of bytes of the object "val" to transmit
	  * const uint16_t &index - Starting index of the object within the
	  transmit buffer (txBuff)

	 Return:
	 -------
	  * uint8_t - Number of bytes written to the transmit buffer (txBuff)
	*/
	template <typename T>
	uint8_t txObj(const T &val, const uint16_t &len=sizeof(T), const uint16_t &index=0)
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

		if ((len + index) > MAX_PACKET_SIZE)
			return MAX_PACKET_SIZE - index;
		else
			return len;
	}




	/*
	 void SerialTransfer::rxObj(const T &val, const uint16_t &len=sizeof(T), const uint16_t &index=0)
	 Description:
	 ------------
	  * Reads "len" number of bytes from the receive buffer (rxBuff)
	  starting at the index as specified by the argument "index"
	  into an arbitrary object (byte, int, float, double, struct, etc...)

	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied into from the
	  receive buffer (rxBuff)
	  * const uint16_t &len - Number of bytes in the object "val" received
	  * const uint16_t &index - Starting index of the object within the
	  receive buffer (rxBuff)

	 Return:
	 -------
	  * uint8_t - Number of bytes read from the receive buffer (rxBuff)
	*/
	template <typename T>
	uint8_t rxObj(const T &val, const uint16_t &len=sizeof(T), const uint16_t &index=0)
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

		if ((len + index) > MAX_PACKET_SIZE)
			return MAX_PACKET_SIZE - index;
		else
			return len;
	}




private: // <<---------------------------------------//private
	enum fsm {
		find_start_byte,
		find_id_byte,
		find_overhead_byte,
		find_payload_len,
		find_payload,
		find_crc,
		find_end_byte
	};
	fsm state = find_start_byte;

	functionPtr* callbacks = NULL;
	uint8_t callbacksLen = 0;

	Stream* port;
	Stream* debugPort;
	bool debug = false;

	uint8_t bytesToRec = 0;
	uint8_t payIndex = 0;
	uint8_t idByte = 0;
	uint8_t overheadByte = 0;
	uint8_t recOverheadByte = 0;





	void calcOverhead(uint8_t arr[], const uint8_t &len);
	int16_t findLast(uint8_t arr[], const uint8_t &len);
	void stuffPacket(uint8_t arr[], const uint8_t &len);
	void unpackPacket(uint8_t arr[], const uint8_t &len);
};
