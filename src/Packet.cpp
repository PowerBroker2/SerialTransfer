#include "Packet.h"


PacketCRC crc;


Packet::Packet(bool debug) : debug(debug)
{
}

void Packet::printDebug(const char* msg)
{
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
uint8_t Packet::constructPacket(uint8_t packetID)
{
	if (bytesToSend > MAX_PACKET_SIZE)
	{
		bytesToSend = MAX_PACKET_SIZE;
	}

	calcOverhead(txBuff, bytesToSend);
	stuffPacket(txBuff, bytesToSend);
	uint8_t crcVal = crc.calculate(txBuff, bytesToSend);

	preamble[1] = packetID;
	preamble[2] = overheadByte;
	preamble[3] = bytesToSend;

	postamble[0] = crcVal;

	return bytesToSend;
}


/*
 uint8_t Packet::available()
 Description:
 ------------
  * Parses incoming serial data, analyzes packet contents,
  and reports errors/successful packet reception
 Inputs:
 -------
  * void
 Return:
 -------
  * uint8_t - Num bytes in RX buffer
*/
uint8_t Packet::available()
{
	if (bytesAvailable())
	{
		while (bytesAvailable())
		{
			uint8_t recChar = readByte();

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
					return 0;
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
					return 0;
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
					return bytesToRec;
				}

				bytesRead = 0;
				status    = STOP_BYTE_ERROR;
				return 0;
				break;
			}

			default:
			{
				if (debug)
				{
					printDebug("ERROR: Undefined state ");
				}

				bytesRead = 0;
				state     = find_start_byte;
				break;
			}
			}
		}
	}
	else
	{
		bytesRead = 0;
		status    = NO_DATA;
		return 0;
	}

	bytesRead = 0;
	status    = CONTINUE;
	return 0;
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
