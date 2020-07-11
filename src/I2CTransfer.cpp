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
void I2CTransfer::begin(TwoWire &_port, const configST configs)
{
	port = &_port;
	port->onReceive(processData);
	packet.begin(configs);
}




/*
 uint8_t I2CTransfer::sendData(const uint16_t &messageLen, const uint8_t packetID)
 Description:
 ------------
  * Send a specified number of bytes in packetized form
 Inputs:
 -------
  * const uint16_t &messageLen - Number of values in txBuff
  to send as the payload in the next packet
  * const uint8_t packetID - The packet 8-bit identifier
 Return:
 -------
  * uint8_t numBytesIncl - Number of payload bytes included in packet
*/
uint8_t I2CTransfer::sendData(const uint16_t &messageLen, const uint8_t packetID)
{
	uint8_t numBytesIncl;

	numBytesIncl = packet.constructPacket(messageLen, packetID);

	port->beginTransmission(packetID);
	port->write(packet.txBuff, numBytesIncl + NUM_OVERHEAD);
	port->endTransmission();

	return numBytesIncl;
}




/*
 uint8_t I2CTransfer::available()
 Description:
 ------------
  * Parses incoming serial data, analyzes packet contents,
  and reports errors/successful packet reception
 Inputs:
 -------
  * void
 Return:
 -------
  * uint8_t bytesRead - Num bytes in RX buffer
*/
uint8_t I2CTransfer::processData()
{
	uint8_t recChar;
	bytesRead = 0;

	while (port->available())
	{
		recChar = port->read();
		bytesRead = packet.parse(recChar);
		status = packet.status;
	}

	return bytesRead;
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
