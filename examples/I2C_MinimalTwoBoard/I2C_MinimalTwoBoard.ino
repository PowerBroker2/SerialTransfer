#include "I2CTransfer.h"

// Upload to both boards. Use 1 on the master and 0 on the slave.
#define I2C_TEST_MASTER 1

static const uint8_t SLAVE_ADDRESS = 0x42;
static const uint8_t COUNTER_PACKET_ID = 1;

I2CTransfer transfer;


void setup()
{
	Serial.begin(115200);

#if I2C_TEST_MASTER
	Wire.begin();
	transfer.begin(Wire, false, Serial);
	Serial.println(F("I2C quickstart: MASTER -> address 0x42"));
#else
	Wire.begin(SLAVE_ADDRESS);
	transfer.begin(Wire, false, Serial);
	Serial.println(F("I2C quickstart: SLAVE at address 0x42"));
#endif

	Serial.println(F("Connect SDA-to-SDA, SCL-to-SCL, and GND-to-GND."));
}


void loop()
{
#if I2C_TEST_MASTER
	static uint32_t counter = 0;
	static uint32_t lastSendMs = 0;
	const uint32_t now = millis();

	if ((uint32_t)(now - lastSendMs) >= 1000)
	{
		lastSendMs = now;
		transfer.txObj(counter);
		const uint8_t sent = transfer.sendData(sizeof(counter), COUNTER_PACKET_ID, SLAVE_ADDRESS);
		Serial.print(F("Sent counter "));
		Serial.print(counter);
		Serial.print(F("; I2C status = "));
		Serial.println(transfer.transmissionStatus);
		if (sent != sizeof(counter))
			Serial.println(F("WARNING: payload was not sent completely."));
		++counter;
	}
#else
	const uint8_t payloadLength = transfer.available();
	if (transfer.status == NEW_DATA)
	{
		if (transfer.currentPacketID() == COUNTER_PACKET_ID && payloadLength == sizeof(uint32_t))
		{
			uint32_t counter = 0;
			transfer.rxObj(counter);
			Serial.print(F("Received counter: "));
			Serial.println(counter);
		}
		else
			Serial.println(F("Received packet with unexpected ID or length."));
	}
#endif
}
