#include "Packet.h"


const PacketCRC<> Packet::crc{};


Packet::Packet(bool debug) : debug(debug)
{
}


Packet::~Packet()
{
	CallbackNode* cur;
	CallbackNode* toDel;

	while (cur != nullptr)
	{
		toDel = cur;
		cur   = cur->next;

		delete toDel;
	}
}


/*
 uint8_t Packet::sendPacket(uint8_t packetID)
 Description:
 ------------
  * Calculate, format, and insert the packet protocol metadata into the packet transmit
  buffer, then send the packet.
 Inputs:
 -------
  * const uint16_t &messageLen - Number of values in txBuff
  to send as the payload in the next packet
  * const uint8_t packetID - The packet 8-bit identifier
 Return:
 -------
  * uint8_t - Number of payload bytes included in packet
*/
uint8_t Packet::sendPacket(uint8_t packetID)
{
	// This should not happen but let's have this just to be save
	if (bytesToSend > MAX_PACKET_SIZE)
		bytesToSend = MAX_PACKET_SIZE;

	// Construct the packet
	uint8_t overheadByte = stuffPacket();
	uint8_t crcVal       = crc.calculate(txBuff, bytesToSend);

	preamble[1] = packetID;
	preamble[2] = overheadByte;
	preamble[3] = bytesToSend;

	postamble[0] = crcVal;

	// Send it off
	writeBytes();

	uint8_t bytesSent = bytesToSend;
	bytesToSend       = 0;

	return bytesSent;
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
			case fsm::find_start_byte: /////////////////////////////////////////
			{
				if (recChar == START_BYTE)
					state = fsm::find_id_byte;
				break;
			}

			case fsm::find_id_byte: ////////////////////////////////////////////
			{
				idByte = recChar;
				state  = fsm::find_overhead_byte;
				break;
			}

			case fsm::find_overhead_byte: //////////////////////////////////////
			{
				recOverheadByte = recChar;
				state           = fsm::find_payload_len;
				break;
			}

			case fsm::find_payload_len: ////////////////////////////////////////
			{
				if (recChar <= MAX_PACKET_SIZE)
				{
					bytesToRec = recChar;
					state      = fsm::find_payload;
				}
				else
				{
					bytesRec = 0;
					state    = fsm::find_start_byte;
					status   = PAYLOAD_ERROR;
					return 0;
				}
				break;
			}

			case fsm::find_payload: ////////////////////////////////////////////
			{
				if (payIndex < bytesToRec)
				{
					rxBuff[payIndex] = recChar;
					payIndex++;

					if (payIndex == bytesToRec)
					{
						payIndex = 0;
						state    = fsm::find_crc;
					}
				}
				break;
			}

			case fsm::find_crc: ///////////////////////////////////////////
			{
				uint8_t calcCrc = crc.calculate(rxBuff, bytesToRec);

				if (calcCrc == recChar)
					state = fsm::find_end_byte;
				else
				{
					bytesRec = 0;
					state    = fsm::find_start_byte;
					status   = CRC_ERROR;
					return 0;
				}

				break;
			}

			case fsm::find_end_byte: ///////////////////////////////////////////
			{
				state = fsm::find_start_byte;

				if (recChar == STOP_BYTE)
				{
					unpackPacket();
					bytesRec  = bytesToRec;
					bytesRead = 0;
					status    = NEW_DATA;

					callCallbacks();

					return bytesToRec;
				}

				bytesRec = 0;
				status   = STOP_BYTE_ERROR;
				return 0;
				break;
			}

			default:
			{
				if (debug)
				{
					printDebug("ERROR: Undefined state ");
				}

				bytesRec = 0;
				state    = fsm::find_start_byte;
				break;
			}
			}
		}
	}
	else
	{
		bytesRec = 0;
		status   = NO_DATA;
		return 0;
	}

	bytesRec = 0;
	status   = CONTINUE;
	return 0;
}


bool Packet::tick()
{
	return available() > 0;
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
uint8_t Packet::getPacketID()
{
	return idByte;
}


uint8_t Packet::getPacketSize()
{
	return bytesRec;
}


ParserState Packet::getStatus()
{
	return status;
}


void Packet::addCallback(CallbackFunc callback)
{
	CallbackNode* newNode = new CallbackNode(callback);

	if (callbacks == nullptr)
	{
		callbacks = newNode;
		return;
	}

	CallbackNode* cur = callbacks;

	while (cur->next != nullptr)
		cur = cur->next;

	cur->next = newNode;
}


void Packet::printDebug(const char* msg)
{
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
uint8_t Packet::stuffPacket()
{
	uint8_t lastPos = 0xFF;

	for (uint8_t i = (bytesToSend - 1); i != 0xFF; i--)
	{
		if (txBuff[i] == START_BYTE)
		{
			txBuff[i] = (lastPos == 0xFF) ? 0 : (lastPos - i);
			lastPos   = i;
		}
	}

	return lastPos;
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
void Packet::unpackPacket()
{
	uint8_t testIndex = recOverheadByte;
	uint8_t delta     = 0;

	if (testIndex <= MAX_PACKET_SIZE)
	{
		while (rxBuff[testIndex])
		{
			delta             = rxBuff[testIndex];
			rxBuff[testIndex] = START_BYTE;
			testIndex += delta;
		}
		rxBuff[testIndex] = START_BYTE;
	}
}

void Packet::callCallbacks()
{
	CallbackNode* cur = callbacks;

	while (cur != nullptr)
	{
		cur->callback(txBuff, bytesRec, idByte);
		cur = cur->next;
	}
}
