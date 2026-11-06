#include "I2CTransfer.h"

// Upload this sketch to both boards. Leave 1 on the master board and set it
// to 0 on the slave board, then upload again.
#define I2C_TEST_MASTER 1

static const uint8_t SLAVE_ADDRESS = 0x42;
static const uint8_t TEST_PACKET_ID = 0x5A;
static const uint8_t TEST_PAYLOAD_SIZE = 255;
static const uint32_t REPORT_INTERVAL_MS = 5000;

I2CTransfer transfer;
uint32_t lastReportMs = 0;

#if I2C_TEST_MASTER
uint8_t payload[TEST_PAYLOAD_SIZE];
uint16_t nextSequence = 0;


void fillTestPayload(uint8_t* data, uint16_t sequence)
{
	data[0] = (uint8_t)sequence;
	data[1] = (uint8_t)(sequence >> 8);
	for (uint16_t i = 2; i < TEST_PAYLOAD_SIZE; ++i)
		data[i] = (uint8_t)(i * 37u + (sequence & 0xFFu) * 13u + (sequence >> 8) * 7u);
}
#endif


#if I2C_TEST_MASTER

uint32_t txFrames = 0;
uint32_t txErrors = 0;

void setup()
{
	Serial.begin(115200);
	Wire.begin();
	transfer.begin(Wire, false, Serial);

	Serial.println(F("SerialTransfer I2C packet test: MASTER"));
	Serial.println(F("Send a 255-byte payload to slave address 0x42."));
	Serial.println(F("Connect SDA-to-SDA, SCL-to-SCL, and GND-to-GND."));
	lastReportMs = millis();
}


void loop()
{
	fillTestPayload(payload, nextSequence);
	transfer.txObj(payload, 0, sizeof(payload));
	const uint8_t sent = transfer.sendData(sizeof(payload), TEST_PACKET_ID, SLAVE_ADDRESS);

	if (sent == TEST_PAYLOAD_SIZE && transfer.transmissionStatus == 0)
		++txFrames;
	else
	{
		++txErrors;
		Serial.print(F("TX error; Wire status = "));
		Serial.println(transfer.transmissionStatus);
	}

	++nextSequence;
	delay(250);

	if ((uint32_t)(millis() - lastReportMs) >= REPORT_INTERVAL_MS)
	{
		Serial.print(F("TX frames: "));
		Serial.print(txFrames);
		Serial.print(F("; TX errors: "));
		Serial.println(txErrors);
		lastReportMs = millis();
	}
}

#else

uint8_t rxPayload[TEST_PAYLOAD_SIZE];
uint16_t expectedSequence = 0;
uint32_t rxFrames = 0;
uint32_t verifiedFrames = 0;
uint32_t badIDs = 0;
uint32_t badLengths = 0;
uint32_t payloadMismatches = 0;
uint32_t sequenceMismatches = 0;
uint32_t sequenceGaps = 0;
uint32_t duplicateOrOldFrames = 0;
uint32_t crcErrors = 0;
uint32_t otherParserErrors = 0;

void printReport()
{
	Serial.println();
	Serial.println(F("=== I2C receiver report ==="));
	Serial.print(F("Received / fully verified: "));
	Serial.print(rxFrames);
	Serial.print(F(" / "));
	Serial.println(verifiedFrames);
	Serial.print(F("Bad ID / bad length / payload mismatch: "));
	Serial.print(badIDs);
	Serial.print(F(" / "));
	Serial.print(badLengths);
	Serial.print(F(" / "));
	Serial.println(payloadMismatches);
	Serial.print(F("Sequence mismatch / missing frames / duplicate-or-old: "));
	Serial.print(sequenceMismatches);
	Serial.print(F(" / "));
	Serial.print(sequenceGaps);
	Serial.print(F(" / "));
	Serial.println(duplicateOrOldFrames);
	Serial.print(F("CRC errors / other parser errors: "));
	Serial.print(crcErrors);
	Serial.print(F(" / "));
	Serial.println(otherParserErrors);
	Serial.println();
}


void setup()
{
	Serial.begin(115200);
	Wire.begin(SLAVE_ADDRESS);
	transfer.begin(Wire, false, Serial);

	Serial.println(F("SerialTransfer I2C packet test: SLAVE"));
	Serial.println(F("Listening at address 0x42 for 255-byte packets."));
	lastReportMs = millis();
}


void loop()
{
	const uint8_t receivedLength = transfer.available();
	if (transfer.status == NEW_DATA)
	{
		++rxFrames;
		const uint8_t packetID = transfer.currentPacketID();

		if (packetID != TEST_PACKET_ID)
			++badIDs;
		if (receivedLength != TEST_PAYLOAD_SIZE)
			++badLengths;

		if (packetID == TEST_PACKET_ID && receivedLength == TEST_PAYLOAD_SIZE)
		{
			transfer.rxObj(rxPayload, 0, TEST_PAYLOAD_SIZE);
			const uint16_t sequence = (uint16_t)rxPayload[0] | ((uint16_t)rxPayload[1] << 8);

			bool payloadMatches = true;
			for (uint16_t i = 2; i < TEST_PAYLOAD_SIZE; ++i)
			{
				const uint8_t expectedByte = (uint8_t)(i * 37u + (sequence & 0xFFu) * 13u + (sequence >> 8) * 7u);
				if (rxPayload[i] != expectedByte)
				{
					payloadMatches = false;
					break;
				}
			}
			if (!payloadMatches)
				++payloadMismatches;

			const uint16_t sequenceDelta = sequence - expectedSequence;
			if (sequenceDelta != 0)
			{
				++sequenceMismatches;
				if (sequenceDelta < 0x8000u)
					sequenceGaps += sequenceDelta;
				else
					++duplicateOrOldFrames;
			}
			expectedSequence = sequence + 1;

			if (payloadMatches && sequenceDelta == 0)
				++verifiedFrames;
		}
	}
	else if (transfer.status <= CRC_ERROR)
	{
		if (transfer.status == CRC_ERROR)
			++crcErrors;
		else
			++otherParserErrors;
	}

	if ((uint32_t)(millis() - lastReportMs) >= REPORT_INTERVAL_MS)
	{
		printReport();
		lastReportMs = millis();
	}
}

#endif
