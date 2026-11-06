#include "Packet.h"


PacketCRC crc;


/*
 void Packet::begin(const configST& configs)
 Description:
 ------------
  * Advanced initializer for the Packet Class
 Inputs:
 -------
  * const configST& configs - Struct that holds config
  values for all possible initialization parameters
 Return:
 -------
  * void
*/
void Packet::begin(const configST& configs)
{
	debugPort    = configs.debugPort ? configs.debugPort : &Serial;
	debug        = configs.debug;
	callbacks    = configs.callbacks;
	callbacksLen = configs.callbacksLen;
	timeout 	 = configs.timeout;
}


/*
 void Packet::begin(const bool& _debug, Stream& _debugPort, const uint32_t& _timeout)
 Description:
 ------------
  * Simple initializer for the Packet Class
 Inputs:
 -------
  * const bool& _debug - Whether or not to print error messages
  * Stream &_debugPort - Serial port to print error messages
  * const uint32_t& _timeout - Number of ms to wait before
  declaring packet parsing timeout
 Return:
 -------
  * void
*/
void Packet::begin(const bool& _debug, Stream& _debugPort, const uint32_t& _timeout)
{
	debugPort = &_debugPort;
	debug     = _debug;
	timeout   = _timeout;
}


uint8_t Packet::constructPacket(const uint16_t& messageLen, const uint8_t& packetID)
{
	const uint8_t payloadLen = (messageLen > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : (uint8_t)messageLen;

	preamble[0] = START_BYTE;
	preamble[1] = packetID;
	preamble[2] = payloadLen;

	uint8_t crcValue = crc.update(0, packetID);
	crcValue = crc.update(crcValue, payloadLen);
	for (uint16_t i = 0; i < payloadLen; ++i)
		crcValue = crc.update(crcValue, txBuff[i]);

	postamble[0] = crcValue;
	txBuff[payloadLen] = crcValue;
	return payloadLen;
}


uint8_t Packet::parse(const uint8_t& recChar, const bool& valid)
{
	bytesRead = 0;
	const bool timeoutEnabled = (timeout != __UINT32_MAX__);

	if (!valid)
	{
		if (state != find_start_byte && timeoutEnabled &&
			(uint32_t)(millis() - lastByteTime) >= timeout)
		{
			resetParserState();
			status = STALE_PACKET_ERROR;
			if (debug)
				debugPort->println("ERROR: STALE PACKET");
		}
		else
			status = NO_DATA;

		return 0;
	}

	uint32_t now = 0;
	if (timeoutEnabled && (state != find_start_byte || recChar == START_BYTE))
	{
		now = millis();
		if (state != find_start_byte && (uint32_t)(now - lastByteTime) >= timeout)
		{
			resetParserState();
			status = STALE_PACKET_ERROR;
			if (debug)
				debugPort->println("ERROR: STALE PACKET");

			// Reuse a start byte that arrived after the timeout as the next frame start.
			if (recChar == START_BYTE)
			{
				state = find_id_byte;
				lastByteTime = now;
			}
			return 0;
		}

		if (state != find_start_byte)
			lastByteTime = now;
	}

	switch (state)
	{
	case find_start_byte:
		if (recChar == START_BYTE)
		{
			state = find_id_byte;
			if (timeoutEnabled)
				lastByteTime = now;
		}
		break;

	case find_id_byte:
		idByte = recChar;
		calcCrc = crc.update(0, recChar);
		state = find_payload_len;
		break;

	case find_payload_len:
		bytesToRec = recChar;
		calcCrc = crc.update(calcCrc, recChar);
		payIndex = 0;
		state = (bytesToRec == 0) ? find_crc : find_payload;
		break;

	case find_payload:
		rxBuff[payIndex] = recChar;
		calcCrc = crc.update(calcCrc, recChar);
		++payIndex;
		if (payIndex == bytesToRec)
			state = find_crc;
		break;

	case find_crc:
		if (calcCrc == recChar)
		{
			const uint8_t payloadLen = bytesToRec;
			resetParserState();
			bytesRead = payloadLen;
			status = NEW_DATA;

			if (callbacks)
			{
				if (idByte < callbacksLen && callbacks[idByte])
					callbacks[idByte]();
				else if (debug)
				{
					debugPort->print(F("ERROR: No callback available for packet ID "));
					debugPort->println(idByte);
				}
			}
			return payloadLen;
		}

		resetParserState();
		status = CRC_ERROR;
		if (debug)
			debugPort->println("ERROR: CRC_ERROR");
		return 0;
	}

	status = CONTINUE;
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


void Packet::resetParserState()
{
	state = find_start_byte;
	bytesToRec = 0;
	payIndex = 0;
	calcCrc = 0;
	lastByteTime = 0;
	bytesRead = 0;
}


void Packet::reset()
{
	memset(txBuff, 0, sizeof(txBuff));
	memset(rxBuff, 0, sizeof(rxBuff));
	idByte = 0;
	resetParserState();
	status = NO_DATA;
}
