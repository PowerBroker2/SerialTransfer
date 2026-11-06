#include "SerialTransfer.h"

// Set this to 1 to force SoftwareSerial, or leave automatic selection enabled.
#ifndef SERIALTRANSFER_USE_SOFTWARE_SERIAL
	#if defined(HAVE_HWSERIAL1) || defined(CORE_TEENSY) || defined(ARDUINO_ARCH_ESP32) \
		|| defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32) \
		|| defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_NRF52) || defined(ARDUINO_ARCH_MBED)
		#define SERIALTRANSFER_USE_SOFTWARE_SERIAL 0
	#else
		#define SERIALTRANSFER_USE_SOFTWARE_SERIAL 1
	#endif
#endif

// Used only by the SoftwareSerial fallback. Upload with this set to 1 on the
// peer board; the other board remains the test sender.
#ifndef SERIALTRANSFER_SOFTWARE_SERIAL_PEER
	#define SERIALTRANSFER_SOFTWARE_SERIAL_PEER 0
#endif

#if SERIALTRANSFER_USE_SOFTWARE_SERIAL
	#include <SoftwareSerial.h>
	static const uint8_t SOFT_RX_PIN = 10;
	static const uint8_t SOFT_TX_PIN = 11;
	static const uint32_t TEST_BAUD = 9600;
	SoftwareSerial testPort(SOFT_RX_PIN, SOFT_TX_PIN);
#else
	static const uint32_t TEST_BAUD = 115200;
	#define testPort Serial1
#endif

static const uint8_t PACKET_ID = 1;
static const uint32_t SEND_INTERVAL_MS = 1000;
static const uint32_t ECHO_TIMEOUT_MS = 2000;

SerialTransfer transfer;
uint32_t nextCounter = 0;
uint32_t expectedCounter = 0;
uint32_t lastSendMs = 0;
bool awaitingEcho = false;
#if SERIALTRANSFER_USE_SOFTWARE_SERIAL && SERIALTRANSFER_SOFTWARE_SERIAL_PEER
uint8_t echoPayload[MAX_PACKET_SIZE];
#endif


void setup()
{
	Serial.begin(115200);
	testPort.begin(TEST_BAUD);
	transfer.begin(testPort, false);

	Serial.println(F("SerialTransfer minimal loopback"));
	Serial.print(F("Test port baud: "));
	Serial.println(TEST_BAUD);
#if SERIALTRANSFER_USE_SOFTWARE_SERIAL
	Serial.println(F("SoftwareSerial is half duplex; use two boards for the echo test."));
	Serial.println(F("Cross-connect pin 11 TX to peer pin 10 RX, and pin 10 TX to peer pin 11 RX; share GND."));
	#if SERIALTRANSFER_SOFTWARE_SERIAL_PEER
		Serial.println(F("This board is the echo peer."));
	#else
		Serial.println(F("This board is the test sender."));
	#endif
#else
	Serial.println(F("Jumper the board's TX1 pin to RX1."));
#endif
}


void loop()
{
	transfer.available();

#if SERIALTRANSFER_USE_SOFTWARE_SERIAL && SERIALTRANSFER_SOFTWARE_SERIAL_PEER
	if (transfer.status == NEW_DATA)
	{
		const uint8_t payloadLength = transfer.bytesRead;
		const uint8_t packetID = transfer.currentPacketID();
		transfer.rxObj(echoPayload, 0, payloadLength);
		transfer.txObj(echoPayload, 0, payloadLength);
		transfer.sendData(payloadLength, packetID);
	}
#else
	if (transfer.status == NEW_DATA)
	{
		if (transfer.currentPacketID() == PACKET_ID && transfer.bytesRead == sizeof(uint32_t))
		{
			uint32_t echoedCounter = 0;
			transfer.rxObj(echoedCounter);
			Serial.print(F("Echo received: "));
			Serial.print(echoedCounter);
			if (awaitingEcho && echoedCounter == expectedCounter)
			{
				Serial.println(F(" (verified)"));
				awaitingEcho = false;
			}
			else
				Serial.println(F(" (unexpected)"));
		}
		else
			Serial.println(F("Received a packet with an unexpected ID or length."));
	}

	const uint32_t now = millis();
	if (awaitingEcho && (uint32_t)(now - lastSendMs) >= ECHO_TIMEOUT_MS)
	{
		Serial.print(F("Loopback timeout for packet "));
		Serial.println(expectedCounter);
		awaitingEcho = false;
	}

	if (!awaitingEcho && (uint32_t)(now - lastSendMs) >= SEND_INTERVAL_MS)
	{
		expectedCounter = nextCounter++;
		transfer.txObj(expectedCounter);
		transfer.sendData(sizeof(expectedCounter), PACKET_ID);
		lastSendMs = now;
		awaitingEcho = true;
		Serial.print(F("Sent: "));
		Serial.println(expectedCounter);
	}
#endif
}
