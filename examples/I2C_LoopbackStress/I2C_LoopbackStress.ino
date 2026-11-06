#include "I2CTransfer.h"

// This is a two-board I2C request/reply test, not a single-board wire jumper.
// Upload to both boards; use 1 on the master and 0 on the slave.
#define I2C_TEST_MASTER 1

static const uint8_t SLAVE_ADDRESS = 0x42;
static const uint8_t TEST_PACKET_ID = 50;
static const uint8_t TEST_PAYLOAD_SIZE = 255;
static const uint8_t I2C_CHUNK_BYTES = SERIALTRANSFER_I2C_CHUNK_SIZE;
static const uint32_t I2C_CLOCK_HZ = 100000UL;
static const uint32_t TEST_DURATION_MS = 10000UL;
static const uint32_t ACK_TIMEOUT_MS = 2000UL;
static const uint32_t REPORT_INTERVAL_MS = 5000UL;
static const uint8_t I2C_BITS_PER_BYTE = 9; // Eight data bits plus ACK/NACK.
static const uint16_t FRAME_BYTES = PREAMBLE_SIZE + TEST_PAYLOAD_SIZE + POSTAMBLE_SIZE;
static const uint16_t FRAME_TRANSACTIONS = (FRAME_BYTES + I2C_CHUNK_BYTES - 1) / I2C_CHUNK_BYTES;

I2CTransfer transfer;


uint8_t payloadPattern(const uint16_t sequence, const uint8_t index)
{
	if (index == 0)
		return (uint8_t)sequence;
	if (index == 1)
		return (uint8_t)(sequence >> 8);
	return (uint8_t)((sequence * 31UL) ^ ((uint16_t)index * 73UL) ^ (sequence >> 3));
}


#if I2C_TEST_MASTER

uint8_t txPayload[TEST_PAYLOAD_SIZE];
uint16_t nextSequence = 0;
uint16_t currentSequence = 0;
uint32_t testStartUs = 0;
uint32_t testStartMs = 0;
uint32_t drainStartMs = 0;
uint32_t lastTxUs = 0;
uint32_t lastTxMs = 0;
uint32_t txFrames = 0;
uint32_t completeFrameWrites = 0;
uint32_t verifiedFrames = 0;
uint32_t i2cErrors = 0;
uint32_t ackPolls = 0;
uint32_t ackChecksumErrors = 0;
uint32_t ackTimeouts = 0;
uint32_t latencySamples = 0;
uint32_t latencyTotalUs = 0;
uint32_t latencyMinUs = 0xFFFFFFFFUL;
uint32_t latencyMaxUs = 0;
bool waitingForAck = false;
bool stopSending = false;
bool reportDone = false;


void sendNextFrame()
{
	currentSequence = nextSequence++;
	for (uint16_t i = 0; i < TEST_PAYLOAD_SIZE; ++i)
		txPayload[i] = payloadPattern(currentSequence, (uint8_t)i);

	transfer.txObj(txPayload);
	lastTxUs = micros();
	lastTxMs = millis();
	const uint8_t sent = transfer.sendData(TEST_PAYLOAD_SIZE, TEST_PACKET_ID, SLAVE_ADDRESS);
	++txFrames;
	if (sent != TEST_PAYLOAD_SIZE || transfer.transmissionStatus != 0)
	{
		++i2cErrors;
		return;
	}
	++completeFrameWrites;
	waitingForAck = true;
}


void pollAck()
{
	++ackPolls;
	const uint8_t bytesReceived = Wire.requestFrom((uint8_t)SLAVE_ADDRESS, (uint8_t)3);
	if (bytesReceived == 3 && Wire.available() >= 3)
	{
		uint8_t response[3];
		response[0] = (uint8_t)Wire.read();
		response[1] = (uint8_t)Wire.read();
		response[2] = (uint8_t)Wire.read();
		const uint16_t sequence = (uint16_t)response[0] | ((uint16_t)response[1] << 8);
		if (crc.calculate(response, 2) != response[2])
			++ackChecksumErrors;
		else if (sequence == currentSequence)
		{
			const uint32_t latencyUs = micros() - lastTxUs;
			++verifiedFrames;
			++latencySamples;
			latencyTotalUs += latencyUs;
			if (latencyUs < latencyMinUs)
				latencyMinUs = latencyUs;
			if (latencyUs > latencyMaxUs)
				latencyMaxUs = latencyUs;
			waitingForAck = false;
		}
	}
	else
	{
		while (Wire.available())
			Wire.read();
	}
}


void printReport(const uint32_t endUs)
{
	const uint32_t elapsedUs = endUs - testStartUs;
	const uint32_t failedFrames = txFrames - verifiedFrames;
	// Count completed frame writes, one address byte per data transaction, and
	// each request/ack transaction. Start/stop timing is excluded.
	const uint32_t estimatedWireBytes = completeFrameWrites * (FRAME_BYTES + FRAME_TRANSACTIONS)
		+ ackPolls * 4UL;
	const float wireBps = elapsedUs
		? ((float)estimatedWireBytes * I2C_BITS_PER_BYTE * 1000000.0f) / elapsedUs
		: 0.0f;
	const float payloadBps = elapsedUs
		? ((float)verifiedFrames * TEST_PAYLOAD_SIZE * 8.0f * 1000000.0f) / elapsedUs
		: 0.0f;
	const float utilization = I2C_CLOCK_HZ
		? (wireBps * 100.0f) / I2C_CLOCK_HZ
		: 0.0f;

	Serial.println();
	Serial.println(F("=== SerialTransfer I2C two-board benchmark ==="));
	Serial.print(F("I2C clock / address: "));
	Serial.print(I2C_CLOCK_HZ);
	Serial.print(F(" Hz / 0x"));
	Serial.println(SLAVE_ADDRESS, HEX);
	Serial.print(F("Payload / frame bytes: "));
	Serial.print(TEST_PAYLOAD_SIZE);
	Serial.print(F(" / "));
	Serial.println(FRAME_BYTES);
	Serial.print(F("Wire chunk bytes / data transactions: "));
	Serial.print(I2C_CHUNK_BYTES);
	Serial.print(F(" / "));
	Serial.println(FRAME_TRANSACTIONS);
	Serial.print(F("Elapsed including final drain (ms): "));
	Serial.println(elapsedUs / 1000UL);
	Serial.print(F("TX attempts / complete frame writes / verified replies / failed: "));
	Serial.print(txFrames);
	Serial.print(F(" / "));
	Serial.print(completeFrameWrites);
	Serial.print(F(" / "));
	Serial.print(verifiedFrames);
	Serial.print(F(" / "));
	Serial.println(failedFrames);
	Serial.print(F("Frame failure rate: "));
	Serial.print(txFrames ? (100.0f * failedFrames / txFrames) : 0.0f, 2);
	Serial.println(F("%"));
	Serial.print(F("Estimated I2C wire rate: "));
	Serial.print(wireBps, 1);
	Serial.println(F(" bit/s (start/stop timing excluded)"));
	Serial.print(F("Verified payload goodput: "));
	Serial.print(payloadBps, 1);
	Serial.println(F(" bit/s"));
	Serial.print(F("Estimated bus utilization: "));
	Serial.print(utilization, 1);
	Serial.println(F("%"));
	Serial.print(F("Latency samples / min / avg / max (us): "));
	Serial.print(latencySamples);
	Serial.print(F(" / "));
	if (latencySamples)
	{
		Serial.print(latencyMinUs);
		Serial.print(F(" / "));
		Serial.print((float)latencyTotalUs / latencySamples, 1);
		Serial.print(F(" / "));
		Serial.println(latencyMaxUs);
	}
	else
		Serial.println(F("no valid latency samples"));
	Serial.print(F("Wire errors / acknowledgement timeouts / ACK polls: "));
	Serial.print(i2cErrors);
	Serial.print(F(" / "));
	Serial.print(ackTimeouts);
	Serial.print(F(" / "));
	Serial.println(ackPolls);
	Serial.print(F("ACK checksum errors: "));
	Serial.println(ackChecksumErrors);
}


void setup()
{
	Serial.begin(115200);
	Wire.begin();
	Wire.setClock(I2C_CLOCK_HZ);
	transfer.begin(Wire, false, Serial);

	Serial.println(F("I2C stress test requires two boards."));
	Serial.println(F("Connect SDA-to-SDA, SCL-to-SCL, GND-to-GND; slave address 0x42."));
	Serial.println(F("Sending maximum 255-byte payloads for 10 seconds..."));
	delay(500);
	testStartUs = micros();
	testStartMs = millis();
}


void loop()
{
	if (reportDone)
		return;

	const uint32_t nowMs = millis();
	if (!stopSending && (uint32_t)(nowMs - testStartMs) >= TEST_DURATION_MS)
	{
		stopSending = true;
		drainStartMs = nowMs;
	}

	if (waitingForAck)
	{
		pollAck();
		if (waitingForAck && (uint32_t)(millis() - lastTxMs) >= ACK_TIMEOUT_MS)
		{
			++ackTimeouts;
			waitingForAck = false;
		}
		delay(1);
	}

	if (stopSending)
	{
		if (!waitingForAck || (uint32_t)(millis() - drainStartMs) >= ACK_TIMEOUT_MS)
		{
			if (waitingForAck)
				++ackTimeouts;
			printReport(micros());
			reportDone = true;
		}
		return;
	}

	if (!waitingForAck)
		sendNextFrame();
}

#else

uint8_t rxPayload[TEST_PAYLOAD_SIZE];
volatile uint16_t acknowledgedSequence = 0xFFFFu;
volatile uint8_t acknowledgedChecksum = 0;
uint16_t expectedSequence = 0;
uint32_t rxFrames = 0;
uint32_t verifiedFrames = 0;
uint32_t crcErrors = 0;
uint32_t parserErrors = 0;
uint32_t badIDs = 0;
uint32_t badLengths = 0;
uint32_t payloadErrors = 0;
uint32_t sequenceMismatches = 0;
uint32_t sequenceGaps = 0;
uint32_t duplicateOrOld = 0;
uint32_t lastReportMs = 0;


void onRequest()
{
	const uint16_t sequence = acknowledgedSequence;
	const uint8_t response[3] = {
		(uint8_t)sequence,
		(uint8_t)(sequence >> 8),
		acknowledgedChecksum
	};
	Wire.write(response, sizeof(response));
}


void printSlaveReport()
{
	Serial.println();
	Serial.println(F("=== I2C slave receive analysis ==="));
	Serial.print(F("CRC-valid received / verified payloads: "));
	Serial.print(rxFrames);
	Serial.print(F(" / "));
	Serial.println(verifiedFrames);
	Serial.print(F("CRC errors / other parser errors: "));
	Serial.print(crcErrors);
	Serial.print(F(" / "));
	Serial.println(parserErrors);
	Serial.print(F("Bad ID / bad length / payload mismatch: "));
	Serial.print(badIDs);
	Serial.print(F(" / "));
	Serial.print(badLengths);
	Serial.print(F(" / "));
	Serial.println(payloadErrors);
	Serial.print(F("Sequence mismatch / gaps / duplicate-or-old: "));
	Serial.print(sequenceMismatches);
	Serial.print(F(" / "));
	Serial.print(sequenceGaps);
	Serial.print(F(" / "));
	Serial.println(duplicateOrOld);
}


void setup()
{
	Serial.begin(115200);
	Wire.begin(SLAVE_ADDRESS);
	transfer.begin(Wire, false, Serial);
	Wire.onRequest(onRequest);
	const uint8_t noAck[2] = {0xFF, 0xFF};
	acknowledgedChecksum = crc.calculate(noAck, sizeof(noAck));
	Serial.println(F("I2C stress test: SLAVE at address 0x42."));
	Serial.println(F("Poll the master Serial monitor for benchmark results."));
	lastReportMs = millis();
}


void loop()
{
	transfer.available();
	if (transfer.status == NEW_DATA)
	{
		++rxFrames;
		const uint8_t packetID = transfer.currentPacketID();
		if (packetID != TEST_PACKET_ID)
			++badIDs;
		if (transfer.bytesRead != TEST_PAYLOAD_SIZE)
			++badLengths;

		if (packetID == TEST_PACKET_ID && transfer.bytesRead == TEST_PAYLOAD_SIZE)
		{
			transfer.rxObj(rxPayload);
			const uint16_t sequence = (uint16_t)rxPayload[0] | ((uint16_t)rxPayload[1] << 8);
			bool matches = true;
			for (uint16_t i = 2; i < TEST_PAYLOAD_SIZE; ++i)
			{
				if (rxPayload[i] != payloadPattern(sequence, (uint8_t)i))
				{
					matches = false;
					break;
				}
			}
			if (!matches)
				++payloadErrors;

			const uint16_t delta = sequence - expectedSequence;
			if (delta)
			{
				++sequenceMismatches;
				if (delta < 0x8000u)
					sequenceGaps += delta;
				else
					++duplicateOrOld;
			}
			expectedSequence = sequence + 1;

			if (matches && delta == 0)
			{
				++verifiedFrames;
				const uint8_t response[2] = {(uint8_t)sequence, (uint8_t)(sequence >> 8)};
				const uint8_t checksum = crc.calculate(response, sizeof(response));
				noInterrupts();
				acknowledgedSequence = sequence;
				acknowledgedChecksum = checksum;
				interrupts();
			}
		}
	}
	else if (transfer.status <= CRC_ERROR)
	{
		if (transfer.status == CRC_ERROR)
			++crcErrors;
		else
			++parserErrors;
	}

	if ((uint32_t)(millis() - lastReportMs) >= REPORT_INTERVAL_MS)
	{
		printSlaveReport();
		lastReportMs = millis();
	}
}

#endif
