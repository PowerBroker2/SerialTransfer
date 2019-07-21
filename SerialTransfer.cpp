#include "SerialTransfer.h"




/*
 void SerialTransfer::begin(Stream &_port)

 Description:
 ------------
  * Constructor for the SerialTransfer Class

 Inputs:
 -------
  * Stream *_port - Pointer to Serial port to
  communicate over

 Return:
 -------
  * void
*/
void SerialTransfer::begin(Stream &_port)
{
	port = &_port;
}




/*
 bool SerialTransfer::sendData(uint8_t messageLen)

 Description:
 ------------
  * Send a specified number of bytes in packetized form

 Inputs:
 -------
  * uint8_t messageLen - Number of values in txBuff to
  send as the payload in the next packet

 Return:
 -------
  * bool - Whether or not messageLen was an acceptable
  value
*/
bool SerialTransfer::sendData(uint8_t messageLen)
{
	if (messageLen <= MAX_PACKET_SIZE)
	{
		uint8_t checksum = findChecksum(txBuff, messageLen);

		port->write(START_BYTE);
		port->write(messageLen);
		port->write(txBuff, messageLen);
		port->write(checksum);
		port->write(STOP_BYTE);

		return true;
	}
	
	return false;
}




/*
 uint8_t SerialTransfer::findChecksum(uint8_t arr[], uint8_t len)

 Description:
 ------------
  * Determine the 8-bit checksum of a given number of elements of
  a given array

 Inputs:
 -------
  * uint8_t arr[] - Array of values the checksum is to be calculated
  over
  * uint8_t len - Number of elements in arr[] to calculate checksum
  over

 Return:
  * uint8_t - 8-bit checksum of the given number of elements of
  the given array
 -------
  *
*/
uint8_t SerialTransfer::findChecksum(uint8_t arr[], uint8_t len)
{
	uint8_t checksum = 0;

	for (uint8_t i = 0; i < len; i++)
		checksum += arr[i];

	checksum = ~checksum;

	return checksum;
}




/*
 int8_t SerialTransfer::available()

 Description:
 ------------
  * Parses incoming serial data, analyzes packet contents,
  and reports errors/successful packet reception

 Inputs:
 -------
  * void

 Return:
 -------
  * int8_t - Error code
*/
int8_t SerialTransfer::available()
{
	if (port->available())
	{
		while (port->available())
		{
			uint8_t recChar = port->read();

			switch (state)
			{
				case find_start_byte://///////////////////////////////////////
				{
					if (recChar == START_BYTE)
						state = find_payload_len;
					break;
				}

				case find_payload_len:////////////////////////////////////////
				{
					if (recChar <= MAX_PACKET_SIZE)
					{
						bytesToRec = recChar;
						state = find_payload;
					}
					else
					{
						state = find_start_byte;
						return PAYLOAD_ERROR;
					}
					break;
				}

				case find_payload:////////////////////////////////////////////
				{
					if (payIndex < bytesToRec)
					{
						rxBuff[payIndex] = recChar;
						payIndex++;

						if (payIndex == bytesToRec)
						{
							payIndex = 0;
							state = find_checksum;
						}
					}
					break;
				}

				case find_checksum:///////////////////////////////////////////
				{
					uint8_t calcChecksum = findChecksum(rxBuff, bytesToRec);

					if (calcChecksum == recChar)
						state = find_stop_byte;
					else
					{
						state = find_start_byte;
						return CHECKSUM_ERROR;
					}
				
					break;
				}

				case find_stop_byte:///////////////////////////////////////////
				{
					state = find_start_byte;

					if (recChar == STOP_BYTE)
						return NEW_DATA;

					return STOP_BYTE_ERROR;
					break;
				}

				default:
				{
					Serial.print("ERROR: Undefined state: ");
					Serial.println(state);
					state = find_start_byte;
					break;
				}
			}
		}
	}
	else
		return NO_DATA;

	return CONTINUE;
}
