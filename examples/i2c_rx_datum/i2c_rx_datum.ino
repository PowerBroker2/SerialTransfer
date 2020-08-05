#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;


/////////////////////////////////////////////////////////////////// Callbacks
void handlePacket(Packet& packet)
{
	packet.rxObj(testStruct);
	Serial.print(testStruct.z);
	Serial.print(testStruct.y);
}
///////////////////////////////////////////////////////////////////


void setup()
{
	Serial.begin(115200);
	Wire.begin(0);

	Wire.begin(0);

	// Serial is the debug serial port. If you don't want to enable debugging, you can remove it
	myTransfer.begin(Wire, Serial);
	myTransfer.addCallback(handlePacket);
}


void loop()
{
	myTransfer.tick();
}
