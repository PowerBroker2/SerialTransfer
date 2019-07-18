#include "SerialTransfer.h"




/*
 bool SerialTransfer::begin(Stream *_port,
                            uint8_t _dataSize,
                            uint8_t _numFields,
                            bool txOnly = false)

 Description:
 ------------
  * Constructor for the SerialTransfer Class

 Inputs:
 -------
  * Stream *_port - Pointer to Serial port to
  communicate over
  * uint8_t _numFields - Number of fields per packet
  * uint8_t _dataSize - Number of bytes per field
  * bool _txOnly - Whether or not data will be
  received

 Return:
 -------
  * bool - Whether or not the number of payload bytes
  is within the required limits
*/
bool SerialTransfer::begin(Stream &_port,
	                       uint8_t _numFields,
	                       uint8_t _dataSize,
	                       bool _txOnly)
{
	port = &_port;
	dataSize = _dataSize;
	numFields = _numFields;
	numPayBytes = numFields * dataSize;
	txOnly = _txOnly;

	if (numPayBytes && (numPayBytes < 256))
	{
		// don't allocate array if only transmitting and not receiving
		if (txOnly)
		{
			uint32_t len = (sizeof(uint8_t *) * numFields) + (sizeof(uint8_t) * dataSize * numFields);
			inBuff = (uint8_t **)malloc(len);
		}

		return true;
	}
	
	return false;
}




/*
 bool SerialTransfer::sendData(uint8_t payload[])

 Description:
 ------------
  * 

 Inputs:
 -------
  * 

 Return:
 -------
  * 
*/
bool SerialTransfer::sendData(uint8_t payload[], uint8_t len)
{
	if (len == numPayBytes)
	{
		uint8_t checksum = 0;

		checksum = findChecksum(payload);

		port->write(START_BYTE);
		port->write((uint8_t)(numPayBytes & 0xFF));
		writePayload(payload, len);
		port->write(checksum);
		port->write(STOP_BYTE);

		return true;
	}

	return false;
}




/*
 void SerialTransfer::writePayload(byte payload[])

 Description:
 ------------
  *

 Inputs:
 -------
  *

 Return:
 -------
  *
*/
void SerialTransfer::writePayload(byte payload[], uint8_t len)
{
	if (len == numPayBytes)
	{
		for (byte i = 0; i < len; i++)
			port->write(payload[i]);
	}
}




/*
 uint8_t SerialTransfer::findChecksum(uint8_t payload[])

 Description:
 ------------
  *

 Inputs:
 -------
  *

 Return:
 -------
  *
*/
uint8_t SerialTransfer::findChecksum(uint8_t payload[])
{
	uint8_t checksum = 0;

	for (byte i = 0; i < (sizeof(payload) / sizeof(payload[0])); i++)
		checksum += payload[i];

	return ~checksum;
}




/*
 int8_t SerialTransfer::available()

 Description:
 ------------
  *

 Inputs:
 -------
  *

 Return:
 -------
  *
*/
int8_t SerialTransfer::available()
{
	if (!txOnly)
	{
		if (port->available())
		{
			while (port->available())
			{
				char recChar = port->read();

				if (startFound)
				{
					if (byteIndex == 0)
					{
						if (recChar != numPayBytes)
							return PAYLOAD_ERROR;
					}
					else if (!payReceived)
					{
						inBuff[fieldIndex][subFieldIndex] = recChar;
						
						subFieldIndex++;
						if (subFieldIndex == dataSize)
						{
							if ((fieldIndex == numFields) && (subFieldIndex == dataSize))
								payReceived = true;

							fieldIndex++;
							subFieldIndex = 0;
						}
					}
					else
					{
						if (byteIndex == (numPayBytes + 2))
						{
							// calculate and test checksum
							// return if bad
						}
						else if (byteIndex == (numPayBytes + 3))
						{
							// test if END_BYTE was found
							// reset everything
							// return
						}
					}

					byteIndex++;
				}
				else
					if (recChar == START_BYTE)
						startFound = true;
			}
		}
		else
			return NO_DATA;
	}
	
	return CONFIG_ERROR;
}
