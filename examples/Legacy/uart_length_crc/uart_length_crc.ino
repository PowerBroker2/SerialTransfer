#include "SerialTransfer.h"

const uint8_t PACKET_ID = 7;

SerialTransfer transfer;

struct Telemetry
{
	uint32_t counter;
	float value;
};

Telemetry outgoing;
Telemetry received;

void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);
	transfer.begin(Serial1, false);
	Serial.println("SerialTransfer Serial1 loopback ready");
}

void loop()
{
	static uint32_t counter = 0;
	static uint32_t lastSend = 0;

	if ((uint32_t)(millis() - lastSend) >= 1000)
	{
		lastSend = millis();
		outgoing.counter = counter++;
		outgoing.value = 12.5f + (outgoing.counter * 0.1f);

		transfer.txObj(outgoing);
		transfer.sendData(sizeof(outgoing), PACKET_ID);

		Serial.print("sent #");
		Serial.println(outgoing.counter);
	}

	transfer.available();
	if (transfer.status == NEW_DATA && transfer.currentPacketID() == PACKET_ID)
	{
		transfer.rxObj(received);
		Serial.print("received #");
		Serial.print(received.counter);
		Serial.print(" value=");
		Serial.println(received.value);
	}
}
