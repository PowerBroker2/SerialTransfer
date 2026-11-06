#include "I2CTransfer.h"


void I2CTransfer::begin(TwoWire& _port, const configST& configs)
{
	port = &_port;
	classToUse = this;

	callbacks = configs.callbacks;
	callbacksLen = configs.callbacksLen;
	debugPort = configs.debugPort ? configs.debugPort : &Serial;
	debug = configs.debug;

	// Packet::parse() runs from Wire's receive interrupt. Keep user callbacks
	// and debug output for available(), which is called in normal loop context.
	configST parserConfigs = configs;
	parserConfigs.callbacks = NULL;
	parserConfigs.callbacksLen = 0;
	parserConfigs.debug = false;
	packet.begin(parserConfigs);

	packetPending = false;
	deferredError = CONTINUE;
	bytesRead = 0;
	status = NO_DATA;
	transmissionStatus = 0;
	port->onReceive(processData);
}


void I2CTransfer::begin(TwoWire& _port, const bool& _debug, Stream& _debugPort)
{
	port = &_port;
	classToUse = this;
	callbacks = NULL;
	callbacksLen = 0;
	debugPort = &_debugPort;
	debug = _debug;
	packet.begin(false, _debugPort, DEFAULT_TIMEOUT);

	packetPending = false;
	deferredError = CONTINUE;
	bytesRead = 0;
	status = NO_DATA;
	transmissionStatus = 0;
	port->onReceive(processData);
}


uint8_t I2CTransfer::sendData(const uint16_t& messageLen, const uint8_t& packetID, const uint8_t& targetAddress)
{
	if (!port)
	{
		transmissionStatus = 4;
		return 0;
	}

	const uint8_t payloadLen = packet.constructPacket(messageLen, packetID);
	const uint16_t frameLen = PREAMBLE_SIZE + (uint16_t)payloadLen + POSTAMBLE_SIZE;
	uint16_t framePos = 0;
	transmissionStatus = 0;

	while (framePos < frameLen)
	{
		const uint8_t chunkLen = (frameLen - framePos > SERIALTRANSFER_I2C_CHUNK_SIZE)
			? SERIALTRANSFER_I2C_CHUNK_SIZE
			: (uint8_t)(frameLen - framePos);
		uint8_t written = 0;

		port->beginTransmission(targetAddress);

		if (framePos < PREAMBLE_SIZE)
		{
			const uint8_t headerLen = ((uint16_t)(PREAMBLE_SIZE - framePos) < chunkLen)
				? (uint8_t)(PREAMBLE_SIZE - framePos)
				: chunkLen;
			written += port->write(packet.preamble + framePos, headerLen);

			const uint8_t payloadChunkLen = chunkLen - headerLen;
			if (payloadChunkLen)
				written += port->write(packet.txBuff, payloadChunkLen);
		}
		else
		{
			const uint16_t payloadPos = framePos - PREAMBLE_SIZE;
			written = port->write(packet.txBuff + payloadPos, chunkLen);
		}

		const uint8_t endStatus = port->endTransmission();
		if (written != chunkLen)
		{
			transmissionStatus = 0xFF;
			return 0;
		}
		if (endStatus != 0)
		{
			transmissionStatus = endStatus;
			return 0;
		}

		framePos += chunkLen;
	}

	return payloadLen;
}


uint8_t I2CTransfer::available()
{
	if (packetPending)
	{
		const uint8_t payloadLen = bytesRead;
		const uint8_t packetID = packet.currentPacketID();

		if (callbacks)
		{
			if (packetID < callbacksLen && callbacks[packetID])
				callbacks[packetID]();
			else if (debug && debugPort)
			{
				debugPort->print(F("ERROR: No callback available for packet ID "));
				debugPort->println(packetID);
			}
		}

		// Keep the payload protected from the receive ISR until callbacks have
		// finished reading rxBuff via rxObj().
		packetPending = false;
		status = NEW_DATA;
		return payloadLen;
	}

	const int8_t error = deferredError;
	if (error <= CRC_ERROR)
	{
		deferredError = CONTINUE;
		status = error;
		reportError(error);
		return 0;
	}

	status = NO_DATA;
	return 0;
}


bool I2CTransfer::tick()
{
	available();
	return status == NEW_DATA;
}


void I2CTransfer::processData(int numBytes)
{
	I2CTransfer* transfer = classToUse;
	if (!transfer || !transfer->port)
		return;

	// There is one receive buffer. If application code has not consumed the
	// previous packet yet, drain this transaction without overwriting it.
	if (transfer->packetPending)
	{
		while (numBytes-- > 0 && transfer->port->available())
			transfer->port->read();
		return;
	}

	transfer->bytesRead = 0;
	while (numBytes-- > 0 && transfer->port->available())
	{
		const int recChar = transfer->port->read();
		if (recChar < 0)
			break;

		const uint8_t parsedBytes = transfer->packet.parse((uint8_t)recChar);
		const int8_t parseStatus = transfer->packet.status;
		transfer->status = parseStatus;

		if (parseStatus == NEW_DATA)
		{
			transfer->bytesRead = parsedBytes;
			transfer->packetPending = true;

			// A normal sendData() call emits exactly one frame. Ignore any extra
			// bytes in this same Wire transaction rather than corrupt rxBuff.
			while (transfer->port->available())
				transfer->port->read();
			break;
		}

		if (parseStatus <= CRC_ERROR)
			transfer->deferredError = parseStatus;
	}
}


void I2CTransfer::reportError(int8_t error)
{
	if (!debug || !debugPort)
		return;

	switch (error)
	{
	case CRC_ERROR:
		debugPort->println(F("ERROR: CRC_ERROR"));
		break;
	case PAYLOAD_ERROR:
		debugPort->println(F("ERROR: PAYLOAD_ERROR"));
		break;
	case STOP_BYTE_ERROR:
		debugPort->println(F("ERROR: STOP_BYTE_ERROR"));
		break;
	case STALE_PACKET_ERROR:
		debugPort->println(F("ERROR: STALE PACKET"));
		break;
	default:
		break;
	}
}


uint8_t I2CTransfer::currentPacketID()
{
	return packet.currentPacketID();
}


void I2CTransfer::reset()
{
	packet.reset();
	bytesRead = 0;
	status = NO_DATA;
	packetPending = false;
	deferredError = CONTINUE;
}


I2CTransfer* I2CTransfer::classToUse = NULL;
