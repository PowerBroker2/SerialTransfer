#include "SerialTransfer.h"

// Designed for Teensy 4.x Serial1. Other boards need an RX ring buffer large
// enough to hold a complete incoming frame while sendData() returns.
const uint32_t TEST_BAUD = 115200UL;

// Exercise the protocol's maximum 255-byte payload (259 bytes on the wire).
#define TEST_PAYLOAD_SIZE 255

// Teensy 4.0-specific: buffer a complete frame while SerialTransfer queues it.
// PJRC requires these extra buffer arrays to have global or static storage.
#if defined(__IMXRT1062__)
uint8_t serial1ReadExtra[512];
uint8_t serial1WriteExtra[512];
#endif

const uint32_t TEST_DURATION_MS = 10000UL;
const uint32_t INTERBYTE_TIMEOUT_MS = 250UL;
const uint32_t DRAIN_TIMEOUT_MS = 2000UL;
const uint8_t PACKET_ID = 42;
const uint8_t UART_BITS_PER_BYTE = 10; // 8N1: start + 8 data + stop
const uint16_t FRAME_BYTES = PREAMBLE_SIZE + TEST_PAYLOAD_SIZE + POSTAMBLE_SIZE;

#if TEST_PAYLOAD_SIZE < 4 || TEST_PAYLOAD_SIZE > 255
#error TEST_PAYLOAD_SIZE must be between 4 and 255 bytes
#endif

SerialTransfer transfer;
uint8_t txPayload[TEST_PAYLOAD_SIZE];
uint8_t rxPayload[TEST_PAYLOAD_SIZE];

uint32_t testStartUs = 0;
uint32_t drainStartUs = 0;
uint32_t lastTxUs = 0;
uint32_t nextSequence = 0;
uint32_t currentSequence = 0;
uint32_t previousRxSequence = 0;
bool havePreviousRxSequence = false;
bool waitingForEcho = false;
bool stopSending = false;
bool reportDone = false;

uint32_t txFrames = 0;
uint32_t txWriteErrors = 0;
uint32_t rxFrames = 0;
uint32_t verifiedFrames = 0;
uint32_t crcErrors = 0;
uint32_t parserTimeouts = 0;
uint32_t otherParserErrors = 0;
uint32_t badIDs = 0;
uint32_t badLengths = 0;
uint32_t payloadErrors = 0;
uint32_t sequenceMismatches = 0;
uint32_t sequenceGaps = 0;
uint32_t duplicateOrOldSequences = 0;
uint32_t unsolicitedFrames = 0;
uint32_t drainTimeouts = 0;

uint32_t latencySamples = 0;
uint32_t latencyTotalUs = 0;
uint32_t latencyMinUs = 0xFFFFFFFFUL;
uint32_t latencyMaxUs = 0;


uint8_t payloadPattern(const uint32_t sequence, const uint8_t index)
{
	if (index < 4)
		return (uint8_t)(sequence >> (8U * index));

	return (uint8_t)((sequence * 31UL) ^ ((uint32_t)index * 73UL) ^ (sequence >> 8));
}


uint32_t readSequence(const uint8_t* payload)
{
	return (uint32_t)payload[0]
		| ((uint32_t)payload[1] << 8)
		| ((uint32_t)payload[2] << 16)
		| ((uint32_t)payload[3] << 24);
}


void recordLatency(const uint32_t latencyUs)
{
	++latencySamples;
	latencyTotalUs += latencyUs;
	if (latencyUs < latencyMinUs)
		latencyMinUs = latencyUs;
	if (latencyUs > latencyMaxUs)
		latencyMaxUs = latencyUs;
}


void pollTransfer()
{
	transfer.available();
	const int8_t packetStatus = transfer.status;

	if (packetStatus == CRC_ERROR)
	{
		++crcErrors;
		waitingForEcho = false;
		return;
	}

	if (packetStatus == STALE_PACKET_ERROR)
	{
		++parserTimeouts;
		waitingForEcho = false;
		return;
	}

	if (packetStatus < 0)
	{
		++otherParserErrors;
		waitingForEcho = false;
		return;
	}

	if (packetStatus != NEW_DATA)
		return;

	++rxFrames;
	if (!waitingForEcho)
	{
		++unsolicitedFrames;
		return;
	}

	recordLatency((uint32_t)(micros() - lastTxUs));

	if (transfer.currentPacketID() != PACKET_ID)
	{
		++badIDs;
		waitingForEcho = false;
		return;
	}

	if (transfer.bytesRead != TEST_PAYLOAD_SIZE)
	{
		++badLengths;
		waitingForEcho = false;
		return;
	}

	transfer.rxObj(rxPayload);
	const uint32_t receivedSequence = readSequence(rxPayload);
	if (receivedSequence != currentSequence)
		++sequenceMismatches;

	if (havePreviousRxSequence)
	{
		const uint32_t expectedSequence = previousRxSequence + 1UL;
		if (receivedSequence > expectedSequence)
			sequenceGaps += receivedSequence - expectedSequence;
		else if (receivedSequence <= previousRxSequence)
			++duplicateOrOldSequences;
	}
	previousRxSequence = receivedSequence;
	havePreviousRxSequence = true;

	bool payloadMatches = true;
	for (uint16_t i = 0; i < TEST_PAYLOAD_SIZE; ++i)
	{
		if (rxPayload[i] != txPayload[i])
		{
			payloadMatches = false;
			break;
		}
	}

	if (payloadMatches)
		++verifiedFrames;
	else
		++payloadErrors;

	waitingForEcho = false;
}


void sendNextFrame()
{
	currentSequence = nextSequence++;
	for (uint16_t i = 0; i < TEST_PAYLOAD_SIZE; ++i)
		txPayload[i] = payloadPattern(currentSequence, (uint8_t)i);

	transfer.txObj(txPayload);
	lastTxUs = micros();
	const uint8_t sent = transfer.sendData(TEST_PAYLOAD_SIZE, PACKET_ID);
	if (sent != TEST_PAYLOAD_SIZE)
	{
		++txWriteErrors;
		return;
	}
	++txFrames;
	waitingForEcho = true;
}


void printReport(const uint32_t endUs)
{
	const uint32_t elapsedUs = endUs - testStartUs;
	const uint32_t failedFrames = txFrames - verifiedFrames;
	const float wireBps = elapsedUs
		? ((float)txFrames * FRAME_BYTES * UART_BITS_PER_BYTE * 1000000.0f) / elapsedUs
		: 0.0f;
	const float payloadBps = elapsedUs
		? ((float)verifiedFrames * TEST_PAYLOAD_SIZE * 8.0f * 1000000.0f) / elapsedUs
		: 0.0f;
	const float utilization = TEST_BAUD
		? (wireBps * 100.0f) / TEST_BAUD
		: 0.0f;

	Serial.println();
	Serial.println(F("=== SerialTransfer loopback benchmark ==="));
	Serial.print(F("Baud / UART format: "));
	Serial.print(TEST_BAUD);
	Serial.println(F(" / 8N1"));
	Serial.print(F("Payload / frame bytes: "));
	Serial.print(TEST_PAYLOAD_SIZE);
	Serial.print(F(" / "));
	Serial.println(FRAME_BYTES);
	Serial.print(F("Elapsed (ms, including final drain): "));
	Serial.println(elapsedUs / 1000UL);

	Serial.print(F("TX frames: "));
	Serial.println(txFrames);
	Serial.print(F("CRC-valid RX frames: "));
	Serial.println(rxFrames);
	Serial.print(F("Verified payloads: "));
	Serial.println(verifiedFrames);
	Serial.print(F("Failed or missing frames: "));
	Serial.println(failedFrames);
	Serial.print(F("Serial write errors: "));
	Serial.println(txWriteErrors);
	Serial.print(F("Frame failure rate: "));
	Serial.print(txFrames ? (100.0f * failedFrames / txFrames) : 0.0f, 2);
	Serial.println(F("%"));

	Serial.print(F("Wire rate (attempted): "));
	Serial.print(wireBps, 1);
	Serial.println(F(" bit/s"));
	Serial.print(F("Payload goodput (verified): "));
	Serial.print(payloadBps, 1);
	Serial.println(F(" bit/s"));
	Serial.print(F("Estimated UART utilization: "));
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

	Serial.print(F("CRC errors / parser timeouts / other parser errors: "));
	Serial.print(crcErrors);
	Serial.print(F(" / "));
	Serial.print(parserTimeouts);
	Serial.print(F(" / "));
	Serial.println(otherParserErrors);
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
	Serial.println(duplicateOrOldSequences);
	Serial.print(F("Unsolicited frames / drain timeouts: "));
	Serial.print(unsolicitedFrames);
	Serial.print(F(" / "));
	Serial.println(drainTimeouts);
}


void setup()
{
	Serial.begin(115200);
	Serial1.begin(TEST_BAUD);

	Serial.println(F("SerialTransfer loopback benchmark"));
#if defined(__IMXRT1062__)
	Serial1.addMemoryForRead(serial1ReadExtra, sizeof(serial1ReadExtra));
	Serial1.addMemoryForWrite(serial1WriteExtra, sizeof(serial1WriteExtra));
#endif
	transfer.begin(Serial1, false, Serial, INTERBYTE_TIMEOUT_MS);

	#if defined(CORE_TEENSY)
	Serial.println(F("Jumper Teensy 4.0 pin 1 (TX1) to pin 0 (RX1)."));
	#else
	Serial.println(F("Jumper the board's TX1 pin to RX1."));
	#endif
	Serial.println(F("Open this USB monitor at 115200 baud."));
	Serial.println(F("Testing 255-byte payloads for 10 seconds..."));
	delay(500);
	testStartUs = micros();
}


void loop()
{
	if (reportDone)
		return;

	pollTransfer();
	const uint32_t now = micros();

	if (!stopSending && (uint32_t)(now - testStartUs) >= TEST_DURATION_MS * 1000UL)
	{
		stopSending = true;
		drainStartUs = now;
	}

	if (stopSending)
	{
		if (!waitingForEcho)
		{
			printReport(now);
			reportDone = true;
			return;
		}

		if ((uint32_t)(now - drainStartUs) >= DRAIN_TIMEOUT_MS * 1000UL)
		{
			++drainTimeouts;
			waitingForEcho = false;
			printReport(now);
			reportDone = true;
		}
		return;
	}

	if (!waitingForEcho)
		sendNextFrame();
}
