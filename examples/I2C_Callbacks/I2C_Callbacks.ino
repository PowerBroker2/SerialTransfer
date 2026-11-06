#include "I2CTransfer.h"

// Upload to both boards. Use 1 on the master and 0 on the slave.
#define I2C_TEST_MASTER 1

static const uint8_t SLAVE_ADDRESS = 0x42;
static const uint8_t COUNTER_ID = 1;
static const uint8_t FLAGS_ID = 2;

I2CTransfer transfer;


#if !I2C_TEST_MASTER

void ignorePacketZero()
{
	// Callback tables are indexed by packet ID.
}


void onCounterPacket()
{
	uint32_t counter = 0;
	transfer.rxObj(counter);
	Serial.print(F("Counter callback: "));
	Serial.println(counter);
}


void onFlagsPacket()
{
	uint16_t flags = 0;
	transfer.rxObj(flags);
	Serial.print(F("Flags callback: 0x"));
	Serial.println(flags, HEX);
}


const functionPtr packetCallbacks[] =
{
	ignorePacketZero,
	onCounterPacket,
	onFlagsPacket
};

#endif


void setup()
{
	Serial.begin(115200);

#if I2C_TEST_MASTER
	Wire.begin();
	transfer.begin(Wire, false, Serial);
	Serial.println(F("I2C callback example: MASTER -> address 0x42"));
#else
	Wire.begin(SLAVE_ADDRESS);
	configST config;
	config.debug = true;
	config.callbacks = packetCallbacks;
	config.callbacksLen = sizeof(packetCallbacks) / sizeof(packetCallbacks[0]);
	transfer.begin(Wire, config);
	Serial.println(F("I2C callback example: SLAVE at address 0x42"));
#endif

	Serial.println(F("Connect SDA-to-SDA, SCL-to-SCL, and GND-to-GND."));
}


void loop()
{
#if I2C_TEST_MASTER
	static uint32_t counter = 0;
	static uint32_t lastCounterMs = 0;
	static uint32_t lastFlagsMs = 0;
	const uint32_t now = millis();

	if ((uint32_t)(now - lastCounterMs) >= 1000)
	{
		lastCounterMs = now;
		transfer.txObj(counter);
		transfer.sendData(sizeof(counter), COUNTER_ID, SLAVE_ADDRESS);
		++counter;
	}

	if ((uint32_t)(now - lastFlagsMs) >= 2500)
	{
		lastFlagsMs = now;
		const uint16_t flags = (uint16_t)(0xA500u | (counter & 0xFFu));
		transfer.txObj(flags);
		transfer.sendData(sizeof(flags), FLAGS_ID, SLAVE_ADDRESS);
	}
#else
	// Callbacks are dispatched here, never from Wire's receive interrupt.
	transfer.available();
#endif
}
