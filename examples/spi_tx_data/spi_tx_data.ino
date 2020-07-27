#include "SPITransfer.h"


SPITransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;

char arr[] = "hello";


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
	///////////////////////////////////////// Stuff buffer with struct
	myTransfer.txObj(testStruct);

	///////////////////////////////////////// Stuff buffer with array
	myTransfer.txObj(arr);

	///////////////////////////////////////// Send buffer
	myTransfer.sendPacket();
	delay(500);
}
