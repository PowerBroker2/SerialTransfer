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
		uint8_t crcVal = crc.calculate(txBuff, messageLen);

		port->write(START_BYTE);
		port->write(overheadByte);
		port->write(messageLen);
		port->write(txBuff, messageLen);
		port->write(crcVal);
		port->write(STOP_BYTE);

		return true;
	}
	
	return false;
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
 int16_t SerialTransfer::findLast(uint8_t arr[], uint8_t len)

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
  * int16_t - Index of first instance of the value START_BYTE within the given
  packet array
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
						state = find_crc;
					}
				}
				break;
			}

			case find_crc:///////////////////////////////////////////
			{
				uint8_t calcCrc = crc.calculate(rxBuff, bytesToRec);

				if (calcCrc == recChar)
					state = find_end_byte;
				else
				{
					bytesRead = 0;
					state = find_start_byte;
					status = CRC_ERROR;
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
bool SerialTransfer::txObj(T &val, uint8_t len, uint8_t index)
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
bool SerialTransfer::rxObj(T &val, uint8_t len, uint8_t index)
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
