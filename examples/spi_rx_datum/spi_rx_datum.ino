#include "SPITransfer.h"


SPITransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;


void setup()
{
	Serial.begin(115200);

	pinMode(MISO, OUTPUT);

	// Serial is the debug serial port. If you don't want to enable debugging, you can remove it
	myTransfer.begin(SPI, Serial);
}


void loop()
{
	myTransfer.rxObj(testStruct);
	Serial.print(testStruct.z);
	Serial.print(testStruct.y);
}
