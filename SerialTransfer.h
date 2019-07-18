/*

01111110 00000000 00000000 00000000 ... 00000000 10000001
|      | |      | |      | |      | | | |      | |______|__Stop byte
|      | |      | |      | |      | | | |______|___________8-bit CRC
|      | |      | |      | |      | |_|____________________Rest of payload
|      | |      | |      | |______|________________________2nd payload byte
|      | |      | |______|_________________________________1st payload byte
|      | |______|__________________________________________# of payload bytes
|______|___________________________________________________Start byte (constant)

*/

#pragma once
#include "Arduino.h"




//incoming serial data/parsing errors/constants
const int8_t NO_DATA        = 0;
const int8_t NEW_DATA       = 1;
const int8_t CHECKSUM_ERROR = -1;
const int8_t PAYLOAD_ERROR  = -2;
const int8_t CONFIG_ERROR   = -3;

const uint8_t START_BYTE     = 0x7E;
const uint8_t STOP_BYTE      = 0x81;




class SerialTransfer
{
public: // <<---------------------------------------//public
	uint8_t **inBuff;




	bool begin(Stream &_port,
		       uint8_t _numFields,
		       uint8_t _dataSize,
		       bool _txOnly);
	bool sendData(uint8_t payload[], uint8_t len);
	int8_t available();




private: // <<---------------------------------------//private
	Stream* port;

	uint8_t dataSize;
	uint8_t numFields;
	int16_t numPayBytes;

	uint16_t byteIndex = 1;
	uint8_t fieldIndex = 0;
	uint8_t subFieldIndex = 0;

	bool startFound = false;
	bool payReceived = false;
	bool txOnly;





	uint8_t findChecksum(uint8_t payload[]);
	void writePayload(byte payload[], uint8_t len);
};
