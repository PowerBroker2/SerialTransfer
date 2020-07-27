#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;

char arr[6];


/////////////////////////////////////////////////////////////////// Callbacks
void handlePacket(Packet& packet)
{
	packet.rxObj(testStruct);
	Serial.print(testStruct.z);
	Serial.print(testStruct.y);
	Serial.print(" | ");

	packet.rxObj(arr);
	Serial.println(arr);
}
///////////////////////////////////////////////////////////////////


void setup()
{
	Serial.begin(115200);
	Wire.begin(0);

	myTransfer.begin(Wire, Serial);
	myTransfer.addCallback(handlePacket);
}


void loop()
{
	myTransfer.tick();
}
