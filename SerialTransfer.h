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




const int8_t CONTINUE         = 2;
const int8_t NEW_DATA         = 1;
const int8_t NO_DATA          = 0;
const int8_t CHECKSUM_ERROR   = -1;
const int8_t PAYLOAD_ERROR    = -2;
const int8_t STOP_BYTE_ERROR  = -3;

const uint8_t START_BYTE      = 0x7E;
const uint8_t STOP_BYTE       = 0x81;

const uint8_t MAX_PACKET_SIZE = 0xFE;




class SerialTransfer
{
public: // <<---------------------------------------//public
	uint8_t txBuff[MAX_PACKET_SIZE];
	uint8_t rxBuff[MAX_PACKET_SIZE];

	uint8_t bytesRead = 0;
	int8_t status     = 0;




	void begin(Stream &_port);
	bool sendData(uint8_t messageLen);
	uint8_t available();




private: // <<---------------------------------------//private
	enum fsm {
		find_start_byte,
		find_overhead_byte,
		find_payload_len,
		find_payload,
		find_checksum,
		find_end_byte
	};
	fsm state = find_start_byte;

	Stream* port;

	uint8_t bytesToRec      = 0;
	uint8_t payIndex        = 0;
	uint8_t overheadByte    = 0;
	uint8_t recOverheadByte = 0;





	uint8_t findChecksum(uint8_t arr[], uint8_t len);
	void calcOverhead(uint8_t arr[],    uint8_t len);
	int16_t findLast(uint8_t arr[],     uint8_t len);
	void stuffPacket(uint8_t arr[],     uint8_t len);
	void unpackPacket(uint8_t arr[],    uint8_t len);
	void writePayload();
};
