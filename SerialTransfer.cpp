#include "SerialTransfer.h"




/*
 void SerialTransfer::begin(Stream &_port)

 Description:
 ------------
  * Constructor for the SerialTransfer Class

 Inputs:
 -------
  * Stream &_port - Pointer to Serial port to
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
 void SerialTransfer::txFloat(float &val, uint8_t index)

 Description:
 ------------
  * Stuffs a float (32bit) into the transmit buffer (txBuff)
  starting at the index as specified by the argument "index"

 Inputs:
 -------
 * float &val - Pointer to the float to be copied to the
  transmit buffer (txBuff)
  * uint8_t index - Starting index of the float within the
  transmit buffer (txBuff)

 Return:
 -------
  * bool - Whether or not the specified index is valid
*/
bool SerialTransfer::txFloat(float &val, uint8_t index)
{
	if (index < (MAX_PACKET_SIZE - sizeof(float) + 1))
	{
		uint8_t* ptr = (uint8_t*)&val;

		for (byte i = index; i < sizeof(float); i++)
		{
			txBuff[i] = *ptr;
			ptr++;
		}

		return true;
	}

	return false;
}




/*
 void SerialTransfer::rxFloat(float &val, uint8_t index)

 Description:
 ------------
  * Recreates a float (32bit) from the contents of the
  receive buffer (rxBuff) starting at the index as specified
  by the argument "index"

 Inputs:
 -------
 * float &val - Pointer to the float to be copied to from
  the receive buffer (rxBuff)
  * uint8_t index - Starting index of the float within the
  receive buffer (rxBuff)

 Return:
 -------
  * bool - Whether or not the specified index is valid
*/
bool SerialTransfer::rxFloat(float &val, uint8_t index)
{
	if (index < (MAX_PACKET_SIZE - sizeof(float) + 1))
	{
		uint8_t* ptr = (uint8_t*)&val;

		for (byte i = index; i < sizeof(float); i++)
		{
			*ptr = txBuff[i];
			ptr++;
		}

		return true;
	}

	return false;
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
		calcOverhead(txBuff, messageLen);
		stuffPacket(txBuff, messageLen);
		uint8_t checksum = findChecksum(txBuff, messageLen);

		port->write(START_BYTE);
		port->write(overheadByte);
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
  * uint8_t len - Number of elements in arr[]
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
 void SerialTransfer::calcOverhead(uint8_t arr[], uint8_t len)

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
  * uint8_t len - Number of elements in arr[]

 Return:
 -------
  * void
*/
void SerialTransfer::calcOverhead(uint8_t arr[], uint8_t len)
{
	overheadByte = 0xFF;

	for (uint8_t i = 0; i < len; i++)
		if (arr[i] == START_BYTE)
			overheadByte = i;
}




/*
 int16_t SerialTransfer::packetStuffing(uint8_t arr[], uint8_t len)

 Description:
 ------------
  * Finds last instance of the value START_BYTE within the given
  packet array

 Inputs:
 -------
  * uint8_t arr[] - Packet array
  * uint8_t len - Number of elements in arr[]

 Return:
 -------
  * int16_t - 
*/
int16_t SerialTransfer::findLast(uint8_t arr[], uint8_t len)
{
	for (uint8_t i = (len - 1); i != 0xFF; i--)
		if (arr[i] == START_BYTE)
			return i;

	return -1;
}




/*
 void SerialTransfer::stuffPacket(uint8_t arr[], uint8_t len)

 Description:
 ------------
  * Enforces the COBS (Consistent Overhead Stuffing) ruleset across
  all bytes in the packet against the value of START_BYTE

 Inputs:
 -------
  * uint8_t arr[] - Array of values to stuff
  * uint8_t len - Number of elements in arr[]

 Return:
 -------
  * void
*/
void SerialTransfer::stuffPacket(uint8_t arr[], uint8_t len)
{
	int16_t refByte = findLast(arr, len);

	if (refByte != -1)
	{
		for (uint8_t i = (len - 1); i != 0xFF; i--)
		{
			if (arr[i] == START_BYTE)
			{
				arr[i] = refByte - i;
				refByte = i;
			}
		}
	}
}




/*
 void SerialTransfer::unpackPacket(uint8_t arr[], uint8_t len)

 Description:
 ------------
  * Unpacks all COBS-stuffed bytes within the array

 Inputs:
 -------
  * uint8_t arr[] - Array of values to unpack
  * uint8_t len - Number of elements in arr[]

 Return:
 -------
  * void
*/
void SerialTransfer::unpackPacket(uint8_t arr[], uint8_t len)
{
	uint8_t testIndex = recOverheadByte;
	uint8_t delta = 0;

	if (testIndex <= MAX_PACKET_SIZE)
	{
		while (arr[testIndex])
		{
			delta = arr[testIndex];
			arr[testIndex] = START_BYTE;
			testIndex += delta;
		}
		arr[testIndex] = START_BYTE;
	}
}




/*
 uint8_t SerialTransfer::available()
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
uint8_t SerialTransfer::available()
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
					state = find_overhead_byte;
				break;
			}

			case find_overhead_byte://////////////////////////////////////
			{
				recOverheadByte = recChar;
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
					bytesRead = 0;
					state = find_start_byte;
					status = PAYLOAD_ERROR;
					return 0;
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
					state = find_end_byte;
				else
				{
					bytesRead = 0;
					state = find_start_byte;
					status = CHECKSUM_ERROR;
					return 0;
				}

				break;
			}

			case find_end_byte:///////////////////////////////////////////
			{
				state = find_start_byte;

				if (recChar == STOP_BYTE)
				{
					unpackPacket(rxBuff, bytesToRec);
					bytesRead = bytesToRec;
					status = NEW_DATA;
					return bytesToRec;
				}

				bytesRead = 0;
				status = STOP_BYTE_ERROR;
				return 0;
				break;
			}

			default:
			{
				Serial.print("ERROR: Undefined state: ");
				Serial.println(state);

				bytesRead = 0;
				state = find_start_byte;
				break;
			}
			}
		}
	}
	else
	{
		bytesRead = 0;
		status = NO_DATA;
		return 0;
	}

	bytesRead = 0;
	status = CONTINUE;
	return 0;
}