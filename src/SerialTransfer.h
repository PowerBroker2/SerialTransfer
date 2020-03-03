/*

01111110 11111111 00000000 00000000 00000000 ... 00000000 10000001
|      | |      | |      | |      | |      | | | |      | |______|__Stop byte
|      | |      | |      | |      | |      | | | |______|___________8-bit CRC
|      | |      | |      | |      | |      | |_|____________________Rest of payload
|      | |      | |      | |      | |______|________________________2nd payload byte
|      | |      | |      | |______|_________________________________1st payload byte
|      | |      | |______|__________________________________________# of payload bytes
|      | |______|___________________________________________________COBS Overhead byte
|______|____________________________________________________________Start byte (constant)

*/

#pragma once
#include "Arduino.h"
#include "CRC.h"




const int8_t CONTINUE         = 2;
const int8_t NEW_DATA         = 1;
const int8_t NO_DATA          = 0;
const int8_t CRC_ERROR        = -1;
const int8_t PAYLOAD_ERROR    = -2;
const int8_t STOP_BYTE_ERROR  = -3;

const uint8_t START_BYTE      = 0x7E;
const uint8_t STOP_BYTE       = 0x81;

const uint8_t MAX_PACKET_SIZE = 0xFE;




class SerialTransfer
{
public: // <<---------------------------------------//public
	CRC crc;

	uint8_t txBuff[MAX_PACKET_SIZE];
	uint8_t rxBuff[MAX_PACKET_SIZE];

	uint8_t bytesRead = 0;
	int8_t status     = 0;




	void begin(Stream &_port);
	bool sendData(uint8_t messageLen);
	uint8_t available();




	/*
	 void SerialTransfer::txObj(T &val, uint8_t len, uint8_t index)

	 Description:
	 ------------
	  * Stuffs "len" number of bytes of an arbitrary object (byte, int,
	  float, double, struct, etc...) into the transmit buffer (txBuff)
	  starting at the index as specified by the argument "index"

	 Inputs:
	 -------
	  * T &val - Pointer to the object to be copied to the
	  transmit buffer (txBuff)
	  * uint8_t len - Number of bytes of the object "val" to transmit
	  * uint8_t index - Starting index of the object within the
	  transmit buffer (txBuff)

	 Return:
	 -------
	  * bool - Whether or not the specified index is valid
	*/
	template <typename T>
	bool txObj(T &val, uint8_t len, uint8_t index=0)
	{
		if (index < (MAX_PACKET_SIZE - len + 1))
		{
			uint8_t* ptr = (uint8_t*)&val;

			for (byte i = index; i < (len + index); i++)
			{
				txBuff[i] = *ptr;
				ptr++;
			}

			return true;
		}

		return false;
	}




	/*
	void SerialTransfer::rxObj(T &val, uint8_t len, uint8_t index)

	 Description:
	 ------------
	  * Reads "len" number of bytes from the receive buffer (rxBuff)
	  starting at the index as specified by the argument "index"
	  into an arbitrary object (byte, int, float, double, struct, etc...)

	 Inputs:
	 -------
	  * T &val - Pointer to the object to be copied into from the
	  receive buffer (rxBuff)
	  * uint8_t len - Number of bytes in the object "val" received
	  * uint8_t index - Starting index of the object within the
	  receive buffer (txBuff)

	 Return:
	 -------
	  * bool - Whether or not the specified index is valid
	*/
	template <typename T>
	bool rxObj(T &val, uint8_t len, uint8_t index=0)
	{
		if (index < (MAX_PACKET_SIZE - len + 1))
		{
			uint8_t* ptr = (uint8_t*)&val;

			for (byte i = index; i < (len + index); i++)
			{
				*ptr = rxBuff[i];
				ptr++;
			}

			return true;
		}

		return false;
	}




private: // <<---------------------------------------//private
	enum fsm {
		find_start_byte,
		find_overhead_byte,
		find_payload_len,
		find_payload,
		find_crc,
		find_end_byte
	};
	fsm state = find_start_byte;

	Stream* port;

	uint8_t bytesToRec = 0;
	uint8_t payIndex = 0;
	uint8_t overheadByte = 0;
	uint8_t recOverheadByte = 0;





	void calcOverhead(uint8_t arr[], uint8_t len);
	int16_t findLast(uint8_t arr[], uint8_t len);
	void stuffPacket(uint8_t arr[], uint8_t len);
	void unpackPacket(uint8_t arr[], uint8_t len);
};
