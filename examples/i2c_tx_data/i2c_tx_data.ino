#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;

char arr[] = "hello";


void setup()
{
	Serial.begin(115200);
	Wire.begin();

	// Serial is the debug serial port. If you don't want to enable debugging, you can remove it
	myTransfer.begin(Wire, Serial);

	testStruct.z = '$';
	testStruct.y = 4.5;
}


void loop()
{
	///////////////////////////////////////// Stuff buffer with struct
	myTransfer.txObj(testStruct);

	///////////////////////////////////////// Stuff buffer with array
	myTransfer.txObj(arr);

	///////////////////////////////////////// Send buffer
	myTransfer.sendPacket();
	delay(500);
}
