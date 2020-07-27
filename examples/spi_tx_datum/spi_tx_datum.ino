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
	pinMode(SS, OUTPUT);
	digitalWrite(SS, HIGH);
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV8);

	// Serial is the debug serial port. If you don't want to enable debugging, you can remove it
	myTransfer.begin(SPI, Serial);
}


void loop()
{
	myTransfer.sendObj(testStruct);
	delay(500);
}
