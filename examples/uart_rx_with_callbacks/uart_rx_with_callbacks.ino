#include "SerialTransfer.h"


SerialTransfer myTransfer;


/////////////////////////////////////////////////////////////////// Callbacks
void handlePacket(const uint8_t packetData[], uint8_t packetSize, uint8_t packetId)
{
	Serial.println("New Packet received!");
	Serial.print("PacketSize: ");
	Serial.println((int)packetSize);
	Serial.print("PacketId: ");
	Serial.println((int)packetId);
	Serial.println("PacketData: ");
	Serial.write(packetData, packetSize);
}
///////////////////////////////////////////////////////////////////


void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);

	myTransfer.begin(Serial1);
	myTransfer.addCallback(handlePacket);
}


void loop()
{
	myTransfer.tick();
}
