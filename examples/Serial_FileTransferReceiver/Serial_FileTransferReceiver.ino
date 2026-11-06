#include "SerialTransfer.h"

static const uint32_t LINK_BAUD = 115200UL;
static const uint8_t FILE_BEGIN_ID = 20;
static const uint8_t FILE_DATA_ID = 21;
static const uint8_t FILE_END_ID = 22;
static const uint8_t FILE_ACK_ID = 23;
static const uint16_t FILE_BUFFER_SIZE = 512;

SerialTransfer transfer;
uint8_t receivedFile[FILE_BUFFER_SIZE];
uint16_t fileSize = 0;
uint16_t nextOffset = 0;
uint32_t expectedHash = 0;
uint32_t runningHash = 2166136261UL;
bool fileActive = false;
bool fileReported = false;


uint16_t get16(const uint8_t* in)
{
	return (uint16_t)in[0] | ((uint16_t)in[1] << 8);
}


uint32_t get32(const uint8_t* in)
{
	return (uint32_t)in[0]
		| ((uint32_t)in[1] << 8)
		| ((uint32_t)in[2] << 16)
		| ((uint32_t)in[3] << 24);
}


void put16(uint8_t* out, const uint16_t value)
{
	out[0] = (uint8_t)value;
	out[1] = (uint8_t)(value >> 8);
}


void sendAck(const uint16_t token, const uint8_t result)
{
	uint8_t ack[3];
	put16(ack, token);
	ack[2] = result;
	transfer.txObj(ack);
	transfer.sendData(sizeof(ack), FILE_ACK_ID);
}


bool isDuplicateChunk(const uint16_t offset, const uint8_t* data, const uint16_t length)
{
	if ((uint32_t)offset + length != nextOffset)
		return false;
	for (uint16_t i = 0; i < length; ++i)
		if (receivedFile[offset + i] != data[i])
			return false;
	return true;
}


void printFileSummary()
{
	Serial.println();
	Serial.println(F("=== File transfer verified ==="));
	Serial.print(F("Bytes received: "));
	Serial.println(nextOffset);
	Serial.print(F("FNV-1a checksum: 0x"));
	Serial.println(runningHash, HEX);
	Serial.print(F("First bytes: "));
	for (uint8_t i = 0; i < 16 && i < nextOffset; ++i)
	{
		if (receivedFile[i] < 16)
			Serial.print('0');
		Serial.print(receivedFile[i], HEX);
		Serial.print(' ');
	}
	Serial.println();
}


void processPacket()
{
	const uint8_t packetID = transfer.currentPacketID();
	const uint8_t payloadLength = transfer.bytesRead;
	const uint8_t* data = transfer.packet.rxBuff;

	if (payloadLength < 2)
		return;
	const uint16_t token = get16(data);
	uint8_t result = 1;

	if (packetID == FILE_BEGIN_ID && payloadLength == 8)
	{
		const uint16_t requestedSize = get16(data + 2);
		if (token && requestedSize <= FILE_BUFFER_SIZE)
		{
			fileSize = requestedSize;
			expectedHash = get32(data + 4);
			nextOffset = 0;
			runningHash = 2166136261UL;
			fileActive = true;
			fileReported = false;
			result = 0;
			Serial.print(F("Receiving "));
			Serial.print(fileSize);
			Serial.println(F(" bytes."));
		}
	}
	else if (packetID == FILE_DATA_ID && fileActive && payloadLength > 4)
	{
		const uint16_t offset = get16(data + 2);
		const uint16_t chunkLength = payloadLength - 4;
		if ((uint32_t)offset + chunkLength <= fileSize)
		{
			if (offset == nextOffset)
			{
				for (uint16_t i = 0; i < chunkLength; ++i)
				{
					receivedFile[offset + i] = data[4 + i];
					runningHash = (runningHash ^ data[4 + i]) * 16777619UL;
				}
				nextOffset += chunkLength;
				result = 0;
			}
			else if (offset < nextOffset && isDuplicateChunk(offset, data + 4, chunkLength))
				result = 0; // Retry after a lost ACK; do not hash the bytes twice.
		}
	}
	else if (packetID == FILE_END_ID && payloadLength == 8 && fileActive)
	{
		const uint16_t claimedSize = get16(data + 2);
		const uint32_t claimedHash = get32(data + 4);
		if (claimedSize == fileSize && claimedHash == expectedHash
			&& nextOffset == fileSize && runningHash == expectedHash)
		{
			result = 0;
			if (!fileReported)
			{
				printFileSummary();
				fileReported = true;
			}
		}
	}

	sendAck(token, result);
}


void setup()
{
	Serial.begin(115200);
	Serial1.begin(LINK_BAUD);
	transfer.begin(Serial1, false, Serial);
	Serial.println(F("Serial file-transfer receiver ready."));
	Serial.println(F("Connect receiver RX1 <- sender TX1, TX1 -> sender RX1, and GND."));
}


void loop()
{
	transfer.available();
	if (transfer.status == NEW_DATA)
		processPacket();
}
