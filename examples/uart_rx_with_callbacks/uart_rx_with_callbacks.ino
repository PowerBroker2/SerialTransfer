#include "SerialTransfer.h"


SerialTransfer myTransfer;


/////////////////////////////////////////////////////////////////// Callbacks
void handlePacket(Packet& packet)
{
	Serial.println("New Packet received!");
	Serial.print("PacketSize: ");
	Serial.println((int)packet.getPacketSize());
	Serial.print("PacketId: ");
	Serial.println((int)packet.getPacketID());
	Serial.println("PacketData: ");
	Serial.write(packet.rxBuff, packet.getPacketSize());
}
///////////////////////////////////////////////////////////////////


void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);

	// Serial is the debug serial port. If you don't want to enable debugging, you can remove it
	myTransfer.begin(Serial1, Serial);
	myTransfer.addCallback(handlePacket);
}


void loop()
{
	myTransfer.tick();
}
