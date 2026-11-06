#include "I2CTransfer.h"

static const uint8_t SLAVE_ADDRESS = 0x42;
static const uint8_t FILE_BEGIN_ID = 20;
static const uint8_t FILE_DATA_ID = 21;
static const uint8_t FILE_END_ID = 22;
static const uint8_t FILE_ACK_BYTES = 4;
static const uint16_t FILE_SIZE = 512;
static const uint16_t DATA_CHUNK_BYTES = 240;
static const uint8_t MAX_ATTEMPTS = 3;
static const uint32_t ACK_TIMEOUT_MS = 1000UL;

I2CTransfer transfer;
uint8_t packetPayload[MAX_PACKET_SIZE];
uint16_t tokenCounter = 0;
uint32_t ackChecksumErrors = 0;
bool transferComplete = false;


uint8_t sampleFileByte(const uint16_t offset)
{
	// Deterministic binary source keeps the example independent of an SD card.
	return (uint8_t)((offset * 73UL) ^ ((offset >> 2) * 19UL) ^ 0x5Au);
}


uint32_t sampleFileHash()
{
	uint32_t hash = 2166136261UL;
	for (uint16_t i = 0; i < FILE_SIZE; ++i)
		hash = (hash ^ sampleFileByte(i)) * 16777619UL;
	return hash;
}


uint16_t nextToken()
{
	++tokenCounter;
	if (tokenCounter == 0)
		++tokenCounter;
	return tokenCounter;
}


void put16(uint8_t* out, const uint16_t value)
{
	out[0] = (uint8_t)value;
	out[1] = (uint8_t)(value >> 8);
}


void put32(uint8_t* out, const uint32_t value)
{
	for (uint8_t i = 0; i < 4; ++i)
		out[i] = (uint8_t)(value >> (8u * i));
}


uint16_t get16(const uint8_t* in)
{
	return (uint16_t)in[0] | ((uint16_t)in[1] << 8);
}


bool sendAndWaitForAck(const uint8_t packetID, const uint8_t* payload,
	const uint8_t payloadLength, const uint16_t token)
{
	for (uint8_t attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt)
	{
		for (uint16_t i = 0; i < payloadLength; ++i)
			transfer.packet.txBuff[i] = payload[i];
		const uint8_t sent = transfer.sendData(payloadLength, packetID, SLAVE_ADDRESS);
		if (sent != payloadLength || transfer.transmissionStatus != 0)
		{
			Serial.print(F("I2C send error, status = "));
			Serial.println(transfer.transmissionStatus);
			delay(60); // Allow the receiver's inter-byte timeout to discard a partial frame.
			continue;
		}

		const uint32_t waitStart = millis();
		while ((uint32_t)(millis() - waitStart) < ACK_TIMEOUT_MS)
		{
			const uint8_t count = Wire.requestFrom((uint8_t)SLAVE_ADDRESS, FILE_ACK_BYTES);
			if (count == FILE_ACK_BYTES && Wire.available() >= FILE_ACK_BYTES)
			{
				uint8_t ack[FILE_ACK_BYTES];
				for (uint8_t i = 0; i < FILE_ACK_BYTES; ++i)
					ack[i] = (uint8_t)Wire.read();
				if (crc.calculate(ack, 3) != ack[3])
					++ackChecksumErrors;
				else if (get16(ack) == token)
					return ack[2] == 0;
			}
			else
				while (Wire.available())
					Wire.read();
			delay(2);
		}

		Serial.print(F("No ACK for packet ID "));
		Serial.print(packetID);
		Serial.print(F("; retry "));
		Serial.print(attempt);
		Serial.print(F("/"));
		Serial.println(MAX_ATTEMPTS);
	}
	return false;
}


bool sendFile()
{
	const uint32_t fileHash = sampleFileHash();
	uint16_t token = nextToken();
	put16(packetPayload, token);
	put16(packetPayload + 2, FILE_SIZE);
	put32(packetPayload + 4, fileHash);
	if (!sendAndWaitForAck(FILE_BEGIN_ID, packetPayload, 8, token))
		return false;

	for (uint16_t offset = 0; offset < FILE_SIZE;)
	{
		const uint16_t remaining = FILE_SIZE - offset;
		const uint8_t chunkLength = (remaining > DATA_CHUNK_BYTES)
			? DATA_CHUNK_BYTES : (uint8_t)remaining;
		token = nextToken();
		put16(packetPayload, token);
		put16(packetPayload + 2, offset);
		for (uint16_t i = 0; i < chunkLength; ++i)
			packetPayload[4 + i] = sampleFileByte(offset + i);
		if (!sendAndWaitForAck(FILE_DATA_ID, packetPayload, 4 + chunkLength, token))
			return false;
		offset += chunkLength;
	}

	token = nextToken();
	put16(packetPayload, token);
	put16(packetPayload + 2, FILE_SIZE);
	put32(packetPayload + 4, fileHash);
	return sendAndWaitForAck(FILE_END_ID, packetPayload, 8, token);
}


void setup()
{
	Serial.begin(115200);
	Wire.begin();
	Wire.setClock(100000UL);
	transfer.begin(Wire, false, Serial);
	Serial.println(F("I2C file-transfer master ready."));
	Serial.println(F("Connect SDA/SCL/GND to the slave at address 0x42."));
}


void loop()
{
	if (transferComplete)
		return;
	transferComplete = true;
	Serial.print(F("Sending generated file: "));
	Serial.print(FILE_SIZE);
	Serial.println(F(" bytes."));
	const bool success = sendFile();
	Serial.println(success
		? F("Slave acknowledged every chunk and verified the file hash.")
		: F("Transfer failed; check the slave, bus, and wiring."));
	Serial.print(F("ACK checksum errors: "));
	Serial.println(ackChecksumErrors);
}
