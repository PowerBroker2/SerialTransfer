#include "I2CTransfer.h"


/*
 void I2CTransfer::begin(TwoWire &_port, configST configs)
 Description:
 ------------
  * Advanced initializer for the I2CTransfer Class
 Inputs:
 -------
  * const TwoWire &_port - I2C port to communicate over
  * const configST configs - Struct that holds config
  values for all possible initialization parameters
 Return:
 -------
  * void
*/
void I2CTransfer::begin(TwoWire& _port, const configST configs)
{
	port = &_port;
	port->onReceive(processData);
	packet.begin(configs);
}


/*
 void I2CTransfer::begin(TwoWire &_port, const bool _debug, Stream &_debugPort)
 Description:
 ------------
  * Simple initializer for the SerialTransfer Class
 Inputs:
 -------
  * const TwoWire &_port - I2C port to communicate over
  * const bool _debug - Whether or not to print error messages
  * const Stream &_debugPort - Serial port to print error messages
 Return:
 -------
  * void
*/
void I2CTransfer::begin(TwoWire& _port, const bool _debug, Stream& _debugPort)
{
	port = &_port;
	packet.begin(_debug, _debugPort);
}


/*
 uint8_t I2CTransfer::sendData(const uint16_t &messageLen, const uint8_t &packetID, const uint8_t &targetAddress=0)
 Description:
 ------------
  * Send a specified number of bytes in packetized form
 Inputs:
 -------
  * const uint16_t &messageLen - Number of values in txBuff
  to send as the payload in the next packet
  * const uint8_t &packetID - The packet 8-bit identifier
  * const uint8_t &targetAddress - I2C address to the device the packet
      will be transmitted to
 Return:
 -------
  * uint8_t numBytesIncl - Number of payload bytes included in packet
*/
uint8_t I2CTransfer::sendData(const uint16_t& messageLen, const uint8_t& packetID, const uint8_t& targetAddress)
{
	uint8_t numBytesIncl;

	numBytesIncl = packet.constructPacket(messageLen, packetID);

	port->beginTransmission(targetAddress);
	port->write(packet.preamble, sizeof(packet.preamble));
	port->write(packet.txBuff, numBytesIncl);
	port->write(packet.postamble, sizeof(packet.postamble));
	port->endTransmission();

	return numBytesIncl;
}


/*
 void I2CTransfer::processData(int numBytes)
 Description:
 ------------
  * Parses incoming serial data automatically when an
  I2C frame is received
 Inputs:
 -------
  * int numBytes - Number of I2C bytes to read (ignored)
 Return:
 -------
  * void
*/
void I2CTransfer::processData(int numBytes)
{
	uint8_t recChar;
	classToUse->bytesRead = 0;

	while (classToUse->port->available())
	{
		recChar               = classToUse->port->read();
		classToUse->bytesRead = classToUse->packet.parse(recChar);
		classToUse->status    = classToUse->packet.status;
		
		if (classToUse->status != CONTINUE)
			break;
	}
}


/*
 uint8_t I2CTransfer::currentPacketID()
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
uint8_t I2CTransfer::currentPacketID()
{
	return packet.currentPacketID();
}


I2CTransfer* I2CTransfer::classToUse = NULL;
