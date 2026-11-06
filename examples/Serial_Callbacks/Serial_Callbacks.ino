#include "SerialTransfer.h"

static const uint8_t COUNTER_ID = 1;
static const uint8_t STATUS_ID = 2;

SerialTransfer transfer;
uint32_t nextCounter = 0;
uint32_t lastCounterSendMs = 0;
uint32_t lastStatusSendMs = 0;


void ignorePacketZero()
{
	// IDs are array indexes, so provide a valid function for unused lower IDs.
}


void onCounterPacket()
{
	uint32_t counter = 0;
	transfer.rxObj(counter);
	Serial.print(F("Counter callback: "));
	Serial.println(counter);
}


void onStatusPacket()
{
	uint16_t statusWord = 0;
	transfer.rxObj(statusWord);
	Serial.print(F("Status callback: 0x"));
	Serial.println(statusWord, HEX);
}


// Each entry maps directly to its packet ID. Keep the array alive globally.
const functionPtr packetCallbacks[] =
{
	ignorePacketZero,
	onCounterPacket,
	onStatusPacket
};


void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);

	configST config;
	config.debug = false;
	config.callbacks = packetCallbacks;
	config.callbacksLen = sizeof(packetCallbacks) / sizeof(packetCallbacks[0]);
	transfer.begin(Serial1, config);

	Serial.println(F("Serial callback example ready. Jumper TX1 to RX1."));
}


void loop()
{
	// Polling parses input and runs callbacks in this loop context.
	transfer.available();

	const uint32_t now = millis();
	if ((uint32_t)(now - lastCounterSendMs) >= 1000)
	{
		lastCounterSendMs = now;
		transfer.txObj(nextCounter);
		transfer.sendData(sizeof(nextCounter), COUNTER_ID);
		++nextCounter;
	}

	if ((uint32_t)(now - lastStatusSendMs) >= 2500)
	{
		lastStatusSendMs = now;
		const uint16_t statusWord = 0xA500u | (uint16_t)(nextCounter & 0xFFu);
		transfer.txObj(statusWord);
		transfer.sendData(sizeof(statusWord), STATUS_ID);
	}
}
