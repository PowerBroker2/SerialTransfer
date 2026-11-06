#include "I2CTransfer.h"

static const uint8_t SLAVE_ADDRESS = 0x42;
static const uint8_t FILE_BEGIN_ID = 20;
static const uint8_t FILE_DATA_ID = 21;
static const uint8_t FILE_END_ID = 22;
static const uint16_t FILE_BUFFER_SIZE = 512;

I2CTransfer transfer;
uint8_t receivedFile[FILE_BUFFER_SIZE];
uint16_t fileSize = 0;
uint16_t nextOffset = 0;
uint32_t expectedHash = 0;
uint32_t runningHash = 2166136261UL;
bool fileActive = false;
bool fileReported = false;

// Updated in loop(); read by Wire's onRequest interrupt.
volatile uint16_t replyToken = 0;
volatile uint8_t replyStatus = 0xFF;
volatile uint8_t replyChecksum = 0;


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


void setReply(const uint16_t token, const uint8_t status)
{
	uint8_t response[3] = {(uint8_t)token, (uint8_t)(token >> 8), status};
	const uint8_t checksum = crc.calculate(response, sizeof(response));
	noInterrupts();
	replyToken = token;
	replyStatus = status;
	replyChecksum = checksum;
	interrupts();
}


void onRequest()
{
	const uint16_t token = replyToken;
	const uint8_t response[4] = {(uint8_t)token, (uint8_t)(token >> 8), replyStatus, replyChecksum};
	Wire.write(response, sizeof(response));
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
	Serial.println(F("=== I2C file transfer verified ==="));
	Serial.print(F("Bytes received: "));
	Serial.println(nextOffset);
	Serial.print(F("FNV-1a checksum: 0x"));
	Serial.println(runningHash, HEX);
	Serial.println(F("Every packet CRC and the complete file checksum matched."));
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
	bool completed = false;

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
			completed = !fileReported;
			fileReported = true;
		}
	}

	// I2C slaves can only transmit when the master requests data. The request
	// callback returns this short raw ACK; file packets still use SerialTransfer.
	setReply(token, result);
	if (completed)
		printFileSummary();
}


void setup()
{
	Serial.begin(115200);
	Wire.begin(SLAVE_ADDRESS);
	transfer.begin(Wire, false, Serial);
	Wire.onRequest(onRequest);
	Serial.println(F("I2C file-transfer slave ready at address 0x42."));
}


void loop()
{
	transfer.available();
	if (transfer.status == NEW_DATA)
		processPacket();
}
