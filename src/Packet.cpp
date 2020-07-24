#include "Packet.h"


PacketCRC crc;


/*
 void Packet::begin(configST configs)
 Description:
 ------------
  * Advanced initializer for the Packet Class
 Inputs:
 -------
  * const configST configs - Struct that holds config
  values for all possible initialization parameters
 Return:
 -------
  * void
*/
void Packet::begin(const configST configs)
{
	debugPort    = configs.debugPort;
	debug        = configs.debug;
	callbacks    = configs.callbacks;
	callbacksLen = configs.callbacksLen;

	// need to give the debug port a kick to get things working for some strange reason...
	debugPort->println();
}


/*
 void Packet::begin(const bool _debug, Stream &_debugPort)
 Description:
 ------------
  * Simple initializer for the Packet Class
 Inputs:
 -------
  * const Stream &_port - Serial port to communicate over
  * const bool _debug - Whether or not to print error messages
  * const Stream &_debugPort - Serial port to print error messages
 Return:
 -------
  * void
*/
void Packet::begin(const bool _debug, Stream& _debugPort)
{
	debugPort = &_debugPort;
	debug     = _debug;
}


/*
 uint8_t Packet::constructPacket(const uint16_t &messageLen, const uint8_t packetID)
 Description:
 ------------
  * Calculate, format, and insert the packet protocol metadata into the packet transmit
  buffer
 Inputs:
 -------
  * const uint16_t &messageLen - Number of values in txBuff
  to send as the payload in the next packet
  * const uint8_t packetID - The packet 8-bit identifier
 Return:
 -------
  * uint8_t - Number of payload bytes included in packet
*/
uint8_t Packet::constructPacket(const uint16_t& messageLen, const uint8_t packetID)
{
	if (messageLen > MAX_PACKET_SIZE)
	{
		calcOverhead(txBuff, MAX_PACKET_SIZE);
		stuffPacket(txBuff, MAX_PACKET_SIZE);
		uint8_t crcVal = crc.calculate(txBuff, MAX_PACKET_SIZE);

		preamble[1] = packetID;
		preamble[2] = overheadByte;
		preamble[3] = MAX_PACKET_SIZE;

		postamble[0] = crcVal;

		return MAX_PACKET_SIZE;
	}
	else
	{
		calcOverhead(txBuff, (uint8_t)messageLen);
		stuffPacket(txBuff, (uint8_t)messageLen);
		uint8_t crcVal = crc.calculate(txBuff, (uint8_t)messageLen);

		preamble[1] = packetID;
		preamble[2] = overheadByte;
		preamble[3] = messageLen;

		postamble[0] = crcVal;

		return (uint8_t)messageLen;
	}
}


/*
 uint8_t Packet::parse(uint8_t recChar, bool valid)
 Description:
 ------------
  * Parses incoming serial data, analyzes packet contents,
  and reports errors/successful packet reception. Executes
  callback functions for parsed packets whos ID has a
  corresponding callback function set via
  "void Packet::begin(const configST configs)"
 Inputs:
 -------
  * void
 Return:
 -------
  * uint8_t - Num bytes in RX buffer
*/
uint8_t Packet::parse(uint8_t recChar, bool valid)
{
	if (valid)
	{
		switch (state)
		{
		case find_start_byte: /////////////////////////////////////////
		{
			if (recChar == START_BYTE)
				state = find_id_byte;
			break;
		}

		case find_id_byte: ////////////////////////////////////////////
		{
			idByte = recChar;
			state  = find_overhead_byte;
			break;
		}

		case find_overhead_byte: //////////////////////////////////////
		{
			recOverheadByte = recChar;
			state           = find_payload_len;
			break;
		}

		case find_payload_len: ////////////////////////////////////////
		{
			if (recChar <= MAX_PACKET_SIZE)
			{
				bytesToRec = recChar;
				state      = find_payload;
			}
			else
			{
				bytesRead = 0;
				state     = find_start_byte;
				status    = PAYLOAD_ERROR;

				if (debug)
					debugPort->println("ERROR: PAYLOAD_ERROR");

				return bytesRead;
			}
			break;
		}

		case find_payload: ////////////////////////////////////////////
		{
			if (payIndex < bytesToRec)
			{
				rxBuff[payIndex] = recChar;
				payIndex++;

				if (payIndex == bytesToRec)
				{
					payIndex = 0;
					state    = find_crc;
				}
			}
			break;
		}

		case find_crc: ///////////////////////////////////////////
		{
			uint8_t calcCrc = crc.calculate(rxBuff, bytesToRec);

			if (calcCrc == recChar)
				state = find_end_byte;
			else
			{
				bytesRead = 0;
				state     = find_start_byte;
				status    = CRC_ERROR;

				if (debug)
					debugPort->println("ERROR: CRC_ERROR");

				return bytesRead;
			}

			break;
		}

		case find_end_byte: ///////////////////////////////////////////
		{
			state = find_start_byte;

			if (recChar == STOP_BYTE)
			{
				unpackPacket(rxBuff, bytesToRec);
				bytesRead = bytesToRec;
				status    = NEW_DATA;

				if (callbacks)
				{
					if (idByte < callbacksLen)
						callbacks[idByte]();
					else if (debug)
					{
						debugPort->print(F("ERROR: No callback available for packet ID "));
						debugPort->println(idByte);
					}
				}

				return bytesToRec;
			}

			bytesRead = 0;
			status    = STOP_BYTE_ERROR;

			if (debug)
				debugPort->println("ERROR: STOP_BYTE_ERROR");

			return bytesRead;
			break;
		}

		default:
		{
			if (debug)
			{
				debugPort->print("ERROR: Undefined state ");
				debugPort->println(state);
			}

			bytesRead = 0;
			state     = find_start_byte;
			break;
		}
		}
	}
	else
	{
		bytesRead = 0;
		status    = NO_DATA;
		return bytesRead;
	}

	bytesRead = 0;
	status    = CONTINUE;
	return bytesRead;
}


/*
 uint8_t Packet::currentPacketID()
 Description:
 ------------
  * Returns the ID of the last parsed packet
 Inputs:
 -------
  * void
 Return:
 -------
  * uint8_t - ID of the last parsed packet
*/
uint8_t Packet::currentPacketID()
{
	return idByte;
}


/*
 void Packet::calcOverhead(uint8_t arr[], const uint8_t &len)
 Description:
 ------------
  * Calculates the COBS (Consistent Overhead Stuffing) Overhead
  byte and stores it in the class's overheadByte variable. This
  variable holds the byte position (within the payload) of the
  first payload byte equal to that of START_BYTE
 Inputs:
 -------
  * uint8_t arr[] - Array of values the overhead is to be calculated
  over
  * const uint8_t &len - Number of elements in arr[]
 Return:
 -------
  * void
*/
void Packet::calcOverhead(uint8_t arr[], const uint8_t& len)
{
	overheadByte = 0xFF;

	for (uint8_t i = 0; i < len; i++)
	{
		if (arr[i] == START_BYTE)
		{
			overheadByte = i;
			break;
		}
	}
}


/*
 int16_t Packet::findLast(uint8_t arr[], const uint8_t &len)
 Description:
 ------------
  * Finds last instance of the value START_BYTE within the given
  packet array
 Inputs:
 -------
  * uint8_t arr[] - Packet array
  * const uint8_t &len - Number of elements in arr[]
 Return:
 -------
  * int16_t - Index of last instance of the value START_BYTE within the given
  packet array
*/
int16_t Packet::findLast(uint8_t arr[], const uint8_t& len)
{
	for (uint8_t i = (len - 1); i != 0xFF; i--)
		if (arr[i] == START_BYTE)
			return i;

	return -1;
}


/*
 void Packet::stuffPacket(uint8_t arr[], const uint8_t &len)
 Description:
 ------------
  * Enforces the COBS (Consistent Overhead Stuffing) ruleset across
  all bytes in the packet against the value of START_BYTE
 Inputs:
 -------
  * uint8_t arr[] - Array of values to stuff
  * const uint8_t &len - Number of elements in arr[]
 Return:
 -------
  * void
*/
void Packet::stuffPacket(uint8_t arr[], const uint8_t& len)
{
	int16_t refByte = findLast(arr, len);

	if (refByte != -1)
	{
		for (uint8_t i = (len - 1); i != 0xFF; i--)
		{
			if (arr[i] == START_BYTE)
			{
				arr[i]  = refByte - i;
				refByte = i;
			}
		}
	}
}


/*
 void Packet::unpackPacket(uint8_t arr[], const uint8_t &len)
 Description:
 ------------
  * Unpacks all COBS-stuffed bytes within the array
 Inputs:
 -------
  * uint8_t arr[] - Array of values to unpack
  * const uint8_t &len - Number of elements in arr[]
 Return:
 -------
  * void
*/
void Packet::unpackPacket(uint8_t arr[], const uint8_t& len)
{
	uint8_t testIndex = recOverheadByte;
	uint8_t delta     = 0;

	if (testIndex <= MAX_PACKET_SIZE)
	{
		while (arr[testIndex])
		{
			delta          = arr[testIndex];
			arr[testIndex] = START_BYTE;
			testIndex += delta;
		}
		arr[testIndex] = START_BYTE;
	}
}
