#include "I2CTransfer.h"


I2CTransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;


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
	myTransfer.sendObj(testStruct);
	delay(500);
}
