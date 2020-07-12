#include "SPITransfer.h"




/*
 void SPITransfer::begin(SPIClass &_port, configST configs, const uint8_t &_SS)
 Description:
 ------------
  * Advanced initializer for the SPITransfer Class
 Inputs:
 -------
  * const SPIClass &_port - SPI port to communicate over
  * const configST configs - Struct that holds config
  * const uint8_t &_SS - SPI buslave select pin used
  values for all possible initialization parameters
 Return:
 -------
  * void
*/
void SPITransfer::begin(SPIClass &_port, const configST configs, const uint8_t &_SS)
{
	port = &_port;
	packet.begin(configs);
	ssPin = _SS;
}




/*
 void SPITransfer::begin(SPIClass &_port, const uint8_t &_SS, const bool _debug, Stream &_debugPort)
 Description:
 ------------
  * Simple initializer for the SPITransfer Class
 Inputs:
 -------
  * const Stream &_port - SPI port to communicate over
  * const uint8_t &_SS - SPI buslave select pin used
  * const bool _debug - Whether or not to print error messages
  * const Stream &_debugPort - Serial port to print error messages
 Return:
 -------
  * void
*/
void SPITransfer::begin(SPIClass &_port, const uint8_t &_SS, const bool _debug, Stream &_debugPort)
{
	port = &_port;
	packet.begin(_debug, _debugPort);
	ssPin = _SS;
}




/*
 uint8_t SPITransfer::sendData(const uint16_t &messageLen, const uint8_t packetID)
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
uint8_t SPITransfer::sendData(const uint16_t &messageLen, const uint8_t packetID)
{
	uint8_t numBytesIncl;

	numBytesIncl = packet.constructPacket(messageLen, packetID);
	
	digitalWrite(SS, LOW);

	for (uint16_t i=0; i<(numBytesIncl + NUM_OVERHEAD); i++)
		port->transfer(packet.txBuff[i]);

	digitalWrite(SS, HIGH);

	return numBytesIncl;
}




/*
 uint8_t SPITransfer::available()
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
uint8_t SPITransfer::available()
{
	uint8_t recChar = SPDR;
	Serial.println(recChar);
	bytesRead = packet.parse(recChar);
	status = packet.status;

	return bytesRead;
}




/*
 bool SPITransfer::tick()
 Description:
 ------------
  * Checks to see if any packets have been fully parsed. This
  is basically a wrapper around the method "available()" and
  is used primarily in conjunction with callbacks
 Inputs:
 -------
  * void
 Return:
 -------
  * bool - Whether or not a full packet has been parsed
*/
bool SPITransfer::tick()
{
	if (available())
		return true;

	return false;
}




/*
 uint8_t SPITransfer::currentPacketID()
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
uint8_t SPITransfer::currentPacketID()
{
	return packet.currentPacketID();
}
